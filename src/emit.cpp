#include "emit.hpp"

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


void Emitter::emit(std::stringstream& out, StrVector& chains) {
  const size_t num_rules = rules_.size();
  const size_t num_trees = trees_.size();
  if (num_rules == 0)
    return;
  const std::string& chain = rules_[0]->chain();
  size_t sub_chain_id = 0;
  std::string sub_chain(chain);
  std::string next_sub_chain(build_chain_name(chain, sub_chain_id));

  size_t i = 0;
  for (size_t j = 0; j < num_trees; ++j) {
    if (j > 0)
      chains.push_back(sub_chain);
    const DomainTuple& domain = domains_[j];
    const size_t start = std::get<0>(domain);
    const size_t end = std::get<1>(domain);
    while (i < start) {
      emit_non_applicable_rule(rules_[i], sub_chain, out);
      ++i;
    }
    i = end + 1;
    const bool leaf_jump = i != num_rules;
    emit_tree(trees_[j], sub_chain, j, next_sub_chain, leaf_jump, out, chains);
    sub_chain = next_sub_chain;
    ++sub_chain_id;
    next_sub_chain = build_chain_name(chain, sub_chain_id);
  }
  if ((i < num_rules) && (num_trees > 0))
    chains.push_back(sub_chain);
  for (; i < num_rules; ++i)
    emit_non_applicable_rule(rules_[i], sub_chain, out);
}


void Emitter::emit_tree(TreeNode* tree,
    const std::string& chain, const size_t tree_id,
    const std::string& next_chain, const bool leaf_jump,
    std::stringstream& out, StrVector& chains) {

  tree->compute_numbering();
  std::string start_chain(build_tree_chain_name(chain, tree_id, tree->id()));
  out << "# Tree " << tree_id << " for Chain " << chain << std::endl;
  out << "-A " << chain << " -j " << start_chain << std::endl;
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
      if (search_ == Arguments::SEARCH_LINEAR)
        emit_simple_linear_dispatch(node, chain, tree_id, node->id(), out,
            chains);
      else if (search_ == Arguments::SEARCH_BINARY)
        emit_simple_binary_dispatch(node, chain, tree_id, node->id(), out,
            chains);
      // now ensure that all children of this node are traversed
      NodeVector& children = node->children();
      const size_t num_children = children.size();
      for (size_t i = 0; i < num_children; ++i)
        node_fifo.push(&children[i]);
    }
  }
  out << std::endl;
}


void emit_linear_ip_dispatch(TreeNode* node, 
    const std::string& chain, const size_t tree_id,
    const size_t chain_count, const std::string& flag,
    const size_t dim, std::stringstream& out, StrVector& chains) {

  NodeVector& children = node->children();
  const size_t num_children = children.size();
  if (num_children == 0)
    return;

  std::string current_chain(build_tree_chain_name(chain, tree_id,
      chain_count));
  out << "# Linear search on " << flag << " ip field, chain " << current_chain
      << std::endl;
  for (size_t i = 0; i < num_children; ++i) {
    TreeNode& child = children[i];
    const DimTuple& range = child.box().box_bounds()[dim];
    std::string jump_chain(build_tree_chain_name(chain, tree_id, child.id()));
    chains.push_back(jump_chain);
    out << "-A " << current_chain
        << " -m iprange --" << flag << "-range "
        << Emitter::num_to_ip(std::get<0>(range)) << "-"
        << Emitter::num_to_ip(std::get<1>(range)) << " -j "
        << jump_chain
        << std::endl;
  }
  out << std::endl;
}


void emit_linear_port_dispatch(TreeNode* node, 
    const std::string& chain, const size_t tree_id,
    const size_t chain_count, const std::string& flag,
    const size_t dim, std::stringstream& out, StrVector& chains) {

  std::string current_chain(build_tree_chain_name(chain, tree_id,
      chain_count));
  out << "# Linear search on " << flag << " port field, chain "
      << current_chain << std::endl;
  NodeVector& children = node->children();
  const size_t num_children = children.size();
  for (size_t i = 0; i < num_children; ++i) {
    TreeNode& child = children[i];
    const DimTuple& range = child.box().box_bounds()[dim];
    std::string jump_chain(build_tree_chain_name(chain, tree_id, child.id()));
    chains.push_back(jump_chain);
    out << "-A " << current_chain
        << " -p " << node->prot() << " --" << flag << " "
        << std::get<0>(range) << ":" << std::get<1>(range) << " -j "
        << jump_chain
        << std::endl;
  }
  out << std::endl;
}


void Emitter::emit_simple_linear_dispatch(TreeNode* node,
    const std::string& chain, const size_t tree_id,
    const size_t chain_count, std::stringstream& out, StrVector& chains) {

  const size_t cut_dim = node->cut_dim();
  switch (cut_dim) {
    // src port
    case 0:
      emit_linear_port_dispatch(node, chain, tree_id, chain_count, "sport",
          cut_dim, out, chains);
      break;
    // dst port
    case 1:
      emit_linear_port_dispatch(node, chain, tree_id, chain_count, "dport",
          cut_dim, out, chains);
      break;
    // src address
    case 2:
      emit_linear_ip_dispatch(node, chain, tree_id, chain_count, "src",
          cut_dim, out, chains);
      break;
    // dst address
    case 3:
      emit_linear_ip_dispatch(node, chain, tree_id, chain_count, "dst",
          cut_dim, out, chains);
  }
}


void emit_binary_port_dispatch(TreeNode* node, const std::string& chain,
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
      out << "-A " << search_chain
          << " -j " << target_chain << std::endl;
    } else {
      // perform the binary dispatch
      // emit test on the lookup HiCuts node
      const TreeNode& lookup_child = hicuts_children[lookup_index];
      std::string target_chain(
          build_tree_chain_name(chain, tree_id, lookup_child.id()));
      chains.push_back(target_chain);
      out << "-A " << search_chain 
          << " -p " << lookup_child.prot()
          << " --" << flag
          << " " << std::get<0>(lookup_child.box().box_bounds()[cut_dim])
          << ":" << std::get<1>(lookup_child.box().box_bounds()[cut_dim])
          << " -j " << target_chain << std::endl;
      // emit test on the left child, if it exists
      if (bin_node->has_left_child()) {
        const BinSearchTree* left_node = bin_node->left();
        target_chain = build_chain_name(current_chain,
            left_node->lookup_index());
        chains.push_back(target_chain);
        Box bbox(TreeNode::minimal_bounding_box(hicuts_children,
            left_node->borders()));
        out << "-A " << search_chain
            << " -p " << lookup_child.prot()
            << " --" << flag
            << " " << std::get<0>(bbox.box_bounds()[cut_dim])
            << ":" << std::get<1>(bbox.box_bounds()[cut_dim])
            << " -j " << target_chain << std::endl;
        // ensure that the search continues on the left child
        fifo.push(bin_node->left());
      }
      // forward to right child
      const BinSearchTree* right_node = bin_node->right();
      target_chain = build_chain_name(current_chain,
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


void emit_binary_ip_dispatch(TreeNode* node, const std::string& chain,
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
      out << "-A " << search_chain
          << " -j " << target_chain << std::endl;
    } else {
      // perform the binary dispatch
      // emit test on the lookup HiCuts node
      const TreeNode& lookup_child = hicuts_children[lookup_index];
      std::string target_chain(
          build_tree_chain_name(chain, tree_id, lookup_child.id()));
      chains.push_back(target_chain);
      out << "-A " << search_chain 
          << " -m iprange "
          << " --" << flag << "-range "
          << " " << Emitter::num_to_ip(
              std::get<0>(lookup_child.box().box_bounds()[cut_dim]))
          << ":" << Emitter::num_to_ip(
              std::get<1>(lookup_child.box().box_bounds()[cut_dim]))
          << " -j " << target_chain << std::endl;
      // emit test on the left child, if it exists
      if (bin_node->has_left_child()) {
        const BinSearchTree* left_node = bin_node->left();
        target_chain = build_chain_name(
            current_chain, left_node->lookup_index());
        chains.push_back(target_chain);
        Box bbox(TreeNode::minimal_bounding_box(hicuts_children,
            left_node->borders()));
        out << "-A " << search_chain
            << " -m iprange "
            << " --" << flag << "-range "
            << " " << Emitter::num_to_ip(
                std::get<0>(bbox.box_bounds()[cut_dim]))
            << ":" << Emitter::num_to_ip(
                std::get<1>(bbox.box_bounds()[cut_dim]))
            << " -j " << target_chain << std::endl;
        // ensure that the search continues on the left child
        fifo.push(bin_node->left());
      }
      // forward to right child
      const BinSearchTree* right_node = bin_node->right();
      target_chain = build_chain_name(current_chain,
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


void Emitter::emit_prefix(std::stringstream& out) {
  out << "*filter" << std::endl
      << ":INPUT ACCEPT [0:0]" << std::endl
      << ":FORWARD ACCEPT [0:0]" << std::endl
      << ":OUTPUT ACCEPT [0:0]" << std::endl;
}


void Emitter::emit_suffix(std::stringstream& out) {
  out << "COMMIT" << std::endl;
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
