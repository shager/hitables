#include "emit.hpp"

/* prototypes */

static void emit_binary_port_dispatch(TreeNode* node, const std::string& chain,
    const size_t tree_id, const size_t chain_count, const std::string& flag,
    const size_t cut_dim, std::stringstream& out, StrVector& chains);

static void emit_binary_ip_dispatch(TreeNode* node, const std::string& chain,
    const size_t tree_id, const size_t chain_count, const std::string& flag,
    const size_t cut_dim, std::stringstream& out, StrVector& chains);

static void emit_port_lookup(const std::string& search_chain,
    const std::string& target_chain, const std::string& flag,
    const TreeNode& lookup_child, const size_t cut_dim,
    const Box& bounding_box, std::stringstream& out);

/* implementation */

std::string build_tree_chain_name(const std::string& chain,
    const size_t tree_id, const size_t chain_id) {

  std::stringstream ss;
  ss << chain << "_" << tree_id << "_" << chain_id;
  return ss.str();
}


std::string build_chain_name(const std::string& chain, const size_t chain_id) {
  std::stringstream ss;
  ss << chain << "_" << chain_id;
  return ss.str();
}


std::string build_bin_search_name(const std::string& chain,
    const size_t tree_id, const size_t chain_id, const size_t bin_search_id) {

  std::stringstream ss;
  ss << chain << "_" << tree_id << "_" << chain_id << "_" << bin_search_id;
  return ss.str();
}


bool is_builtin_chain(const std::string& chain) {
  const bool is_input = chain == "INPUT";
  const bool is_output = chain == "OUTPUT";
  const bool is_forward = chain == "FORWARD";
  return (is_input || is_output || is_forward);
}


void Emitter::emit(std::stringstream& out, StrVector& chains,
    const DefaultPolicies& policies) {

  const size_t num_rules = rules_.size();
  const size_t num_trees = trees_.size();
  if (num_rules == 0)
    return;
  const std::string& chain = rules_[0]->chain();
  size_t sub_chain_id = 0;
  std::string sub_chain(build_chain_name(chain, sub_chain_id));
  ++sub_chain_id;
  std::string next_sub_chain(build_chain_name(chain, sub_chain_id));
  out << "-A " << chain << " -j " << sub_chain << std::endl;
  const bool builtin_chain = is_builtin_chain(chain);

  size_t i = 0;
  for (size_t j = 0; j < num_trees; ++j) {
    chains.push_back(sub_chain);
    const DomainTuple& domain = domains_[j];
    const size_t start = std::get<0>(domain);
    const size_t end = std::get<1>(domain);
    while (i < start) {
      emit_non_applicable_rule(rules_[i], sub_chain, out);
      ++i;
    }
    i = end + 1;
    const bool leaf_jump = builtin_chain || (i != num_rules);
    emit_tree(trees_[j], sub_chain, j, next_sub_chain, leaf_jump, out, chains);
    sub_chain = next_sub_chain;
    ++sub_chain_id;
    next_sub_chain = build_chain_name(chain, sub_chain_id);
  }
  // remember the last index before emitting rules behind the last tree
  const size_t last_i = i;
  for (; i < num_rules; ++i) {
    emit_non_applicable_rule(rules_[i], sub_chain, out);
  }
  // if we have a builtin chain, emit a custom "default" policy as last rule in
  // the chain
  if (builtin_chain) {
    chains.push_back(sub_chain);
    emit_custom_default_rule(sub_chain, policies.chain_policy(chain), out);
  } else
    // remember the name of the last chain only if there were rules in it
    if ((last_i < num_rules) && (num_trees > 0)) {
      chains.push_back(sub_chain);
    }
}


void Emitter::emit_tree(TreeNode* tree,
    const std::string& chain, const size_t tree_id,
    const std::string& next_chain, const bool leaf_jump,
    std::stringstream& out, StrVector& chains) {

  tree->compute_numbering();
  std::string start_chain(build_tree_chain_name(chain, tree_id, tree->id()));
  out << "# Tree " << tree_id << " for Chain " << chain << std::endl;
  out << "-A " << chain << " -p " << tree->prot() 
      << " -j " << start_chain << std::endl;
  // default bail out to next chain if packet does not match tree
  out << "-A " << chain << " -j " << next_chain << std::endl;

  chains.push_back(start_chain);
  NodeRefQueue node_fifo;
  node_fifo.push(tree);

  while (!node_fifo.empty()) {
    TreeNode* node = node_fifo.front();
    node_fifo.pop();
    if (node->is_leaf())
      emit_leaf(node, build_tree_chain_name(chain, tree_id, node->id()),
          next_chain, leaf_jump, out);
    else {
      // emit the dispatch to child nodes
      emit_simple_binary_dispatch(node, chain, tree_id, node->id(), out,
          chains);
      // now ensure that all children of this node are traversed
      NodeVector& children = node->children();
      const size_t num_children = children.size();
      for (size_t i = 0; i < num_children; ++i) {
        node_fifo.push(&children[i]);
      }
    }
  }
  out << std::endl;
}


void Emitter::emit_simple_binary_dispatch(TreeNode* node,
    const std::string& chain, const size_t tree_id,
    const size_t chain_count, std::stringstream& out, StrVector& chains) {

  const size_t cut_dim = node->cut_dim();
  switch (cut_dim) {
    // src port
    case 0:
      emit_binary_port_dispatch(node, chain, tree_id, chain_count, "sport",
          cut_dim, out, chains);
      break;
    // dst port
    case 1:
      emit_binary_port_dispatch(node, chain, tree_id, chain_count, "dport",
          cut_dim, out, chains);
      break;
    // src address
    case 2:
      emit_binary_ip_dispatch(node, chain, tree_id, chain_count, "src",
          cut_dim, out, chains);
      break;
    // dst address
    case 3:
      emit_binary_ip_dispatch(node, chain, tree_id, chain_count, "dst",
          cut_dim, out, chains);
  }
}


static void emit_binary_port_dispatch(TreeNode* node, const std::string& chain,
    const size_t tree_id, const size_t chain_count, const std::string& flag,
    const size_t cut_dim, std::stringstream& out, StrVector& chains) {

  std::string search_chain(build_tree_chain_name(chain, tree_id, chain_count));
  std::string current_chain(search_chain);
  out << "# Binary search on " << flag << ", chain " << chain << std::endl;
  BinSearchTree bin_tree(0, node->num_children() - 1);
  bool at_first_search_node = true;
  NodeVector& hicuts_children = node->children();

  std::queue<const BinSearchTree*> fifo;
  fifo.push(&bin_tree);
  while (!fifo.empty()) {
    const BinSearchTree* bin_node = fifo.front();
    fifo.pop();
    const size_t lookup_index = bin_node->lookup_index();
    if (!at_first_search_node)
      search_chain = build_chain_name(current_chain, lookup_index);
    if (bin_node->is_leaf()) {
      // base case => forward to next HiCuts node
      std::string target_chain(build_tree_chain_name(chain, tree_id,
          hicuts_children[lookup_index].id()));
      chains.push_back(target_chain);
      out << "# binary search leaf node" << std::endl;
      out << "-A " << search_chain
          << " -j " << target_chain << std::endl;
    } else {
      // perform the binary dispatch
      // emit test on the lookup HiCuts node
      const TreeNode& lookup_child = hicuts_children[lookup_index];
      std::string target_chain(
          build_tree_chain_name(chain, tree_id, lookup_child.id()));
      chains.push_back(target_chain);
      out << "# check if binary search terminates" << std::endl;
      emit_port_lookup(search_chain, target_chain, flag, lookup_child, cut_dim,
          lookup_child.box(), out);
      ////out << "-A " << search_chain 
      ////    << " -p " << lookup_child.prot()
      ////    << " --" << flag
      ////    << " " << std::get<0>(lookup_child.box().box_bounds()[cut_dim])
      ////    << ":" << std::get<1>(lookup_child.box().box_bounds()[cut_dim])
      ////    << " -j " << target_chain << std::endl;
      // emit test on the left child, if it exists
      if (bin_node->has_left_child()) {
        const BinSearchTree* left_node = bin_node->left();
        target_chain = build_bin_search_name(chain, tree_id, chain_count,
            left_node->lookup_index());
        chains.push_back(target_chain);
        Box bbox(TreeNode::minimal_bounding_box(hicuts_children,
            left_node->borders()));
        out << "# binary search left branch" << std::endl;
        emit_port_lookup(search_chain, target_chain, flag, lookup_child,
            cut_dim, bbox, out);
        ////out << "-A " << search_chain
        ////    << " -p " << lookup_child.prot()
        ////    << " --" << flag
        ////    << " " << std::get<0>(bbox.box_bounds()[cut_dim])
        ////    << ":" << std::get<1>(bbox.box_bounds()[cut_dim])
        ////    << " -j " << target_chain << std::endl;
        // ensure that the search continues on the left child
        fifo.push(bin_node->left());
      }
      // forward to right child
      out << "# binary search right branch" << std::endl;
      const BinSearchTree* right_node = bin_node->right();
      target_chain = build_bin_search_name(chain, tree_id, chain_count,
          right_node->lookup_index());
      chains.push_back(target_chain);
      out << "-A " << search_chain
          << " -j " << target_chain << std::endl;
      // ensure that the search continues on the right child
      fifo.push(bin_node->right());
    }
    at_first_search_node = false;
  }
  out << std::endl;
}


static void emit_binary_ip_dispatch(TreeNode* node, const std::string& chain,
    const size_t tree_id, const size_t chain_count, const std::string& flag,
    const size_t cut_dim, std::stringstream& out, StrVector& chains) {

  std::string search_chain(build_tree_chain_name(chain, tree_id, chain_count));
  std::string current_chain(search_chain);
  out << "# Binary search on " << flag << ", chain " << chain << std::endl;
  BinSearchTree bin_tree(0, node->num_children() - 1);
  bool at_first_search_node = true;
  NodeVector& hicuts_children = node->children();

  std::queue<const BinSearchTree*> fifo;
  fifo.push(&bin_tree);
  while (!fifo.empty()) {
    const BinSearchTree* bin_node = fifo.front();
    fifo.pop();
    const size_t lookup_index = bin_node->lookup_index();
    if (!at_first_search_node)
      search_chain = build_chain_name(current_chain, lookup_index);
    if (bin_node->is_leaf()) {
      // base case => forward to next HiCuts node
      std::string target_chain(build_tree_chain_name(chain,
          tree_id, hicuts_children[lookup_index].id()));
      chains.push_back(target_chain);
      out << "# binary search leaf node" << std::endl;
      out << "-A " << search_chain
          << " -j " << target_chain << std::endl;
    } else {
      // perform the binary dispatch
      // emit test on the lookup HiCuts node
      out << "# check if binary search terminates" << std::endl;
      const TreeNode& lookup_child = hicuts_children[lookup_index];
      std::string target_chain(
          build_tree_chain_name(chain, tree_id, lookup_child.id()));
      chains.push_back(target_chain);
      out << "-A " << search_chain 
          << " -m iprange "
          << " --" << flag << "-range "
          << " " << Emitter::num_to_ip(
              std::get<0>(lookup_child.box().box_bounds()[cut_dim]))
          << "-" << Emitter::num_to_ip(
              std::get<1>(lookup_child.box().box_bounds()[cut_dim]))
          << " -j " << target_chain << std::endl;
      // emit test on the left child, if it exists
      if (bin_node->has_left_child()) {
        out << "# binary search left branch" << std::endl;
        const BinSearchTree* left_node = bin_node->left();
        target_chain = build_bin_search_name(chain, tree_id, chain_count,
            left_node->lookup_index());
        chains.push_back(target_chain);
        Box bbox(TreeNode::minimal_bounding_box(hicuts_children,
            left_node->borders()));
        out << "-A " << search_chain
            << " -m iprange "
            << " --" << flag << "-range "
            << " " << Emitter::num_to_ip(
                std::get<0>(bbox.box_bounds()[cut_dim]))
            << "-" << Emitter::num_to_ip(
                std::get<1>(bbox.box_bounds()[cut_dim]))
            << " -j " << target_chain << std::endl;
        // ensure that the search continues on the left child
        fifo.push(bin_node->left());
      }
      // forward to right child
      out << "# binary search right branch" << std::endl;
      const BinSearchTree* right_node = bin_node->right();
      target_chain = build_bin_search_name(chain, tree_id, chain_count,
          right_node->lookup_index());
      chains.push_back(target_chain);
      out << "-A " << search_chain
          << " -j " << target_chain << std::endl;
      // ensure that the search continues on the right child
      fifo.push(bin_node->right());
    }
    at_first_search_node = false;
  }
  out << std::endl;
}


void Emitter::emit_leaf(const TreeNode* node, const std::string& current_chain,
    const std::string& next_chain, const bool leaf_jump,
    std::stringstream& out) {
  
  const size_t num_rules = node->num_rules();
  if (num_rules == 0)
    return;
  out << "# leaf node" << std::endl;
  for (size_t i = 0; i < num_rules; ++i) {
    out << node->rules()[i]->src_with_patched_chain(current_chain)
        << std::endl;
  }
  if (leaf_jump)
    out << "-A " << current_chain << " -j " << next_chain << std::endl;
  out << std::endl;
}


static void emit_port_lookup(const std::string& search_chain,
    const std::string& target_chain, const std::string& flag,
    const TreeNode& lookup_child, const size_t cut_dim,
    const Box& bounding_box, std::stringstream& out) {
  
  out << "-A " << search_chain 
      << " -p " << lookup_child.prot()
      << " --" << flag
      << " " << std::get<0>(bounding_box.box_bounds()[cut_dim])
      << ":" << std::get<1>(bounding_box.box_bounds()[cut_dim])
      << " -j " << target_chain << std::endl;
}


void emit_policy(std::ofstream& out, const std::string& chain,
    const ActionCode code) {

  switch (code) {
    case ACCEPT:
      out << ":" << chain <<  " ACCEPT [0:0]" << std::endl;
      break;
    case REJECT:
      out << ":" << chain <<  " REJECT [0:0]" << std::endl;
      break;
    case DROP:
      out << ":" << chain << " DROP [0:0]" << std::endl;
      break;
    default:
      break;
  }
}


void Emitter::emit_prefix(std::ofstream& out,
    const DefaultPolicies& policies) {

  out << "*filter" << std::endl;
  emit_policy(out, "INPUT", policies.input_policy());
  emit_policy(out, "FORWARD", policies.forward_policy());
  emit_policy(out, "OUTPUT", policies.output_policy());
}


void Emitter::emit_suffix(std::ofstream& out) {
  out << "COMMIT" << std::endl;
}


void Emitter::emit_custom_default_rule(const std::string& chain,
    const ActionCode code, std::stringstream& out) {

  out << "-A " << chain << " -j ";
  switch (code) {
    case DROP:
      out << "DROP";
      break;
    case ACCEPT:
      out << "ACCEPT";
      break;
    case REJECT:
      out << "REJECT";
      break;
    default:
      break;
  }
  out << std::endl;
}


std::string Emitter::num_to_ip(const dim_t ip_num) {
  char buf[16];
  sprintf(buf, "%d.%d.%d.%d", ((ip_num & 0xFF000000) >> 24),
                              ((ip_num & 0x00FF0000) >> 16),
                              ((ip_num & 0x0000FF00) >>  8),
                               (ip_num & 0x000000FF));
  return std::string(buf);
}


void Emitter::emit_non_applicable_rule(const Rule* rule,
    const std::string& chain, std::stringstream& out) {

  out << rule->src_with_patched_chain(chain) << std::endl;
}
