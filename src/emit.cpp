#include "emit.hpp"

void Emitter::emit(std::stringstream& out) {
  emit_prefix(out);
  const size_t num_rules = rules_.size();
  const size_t num_trees = trees_.size();
  size_t i = 0;
  for (size_t j = 0; j < num_trees; ++j) {
    const DomainTuple& domain = domains_[j];
    const size_t start = std::get<0>(domain);
    const size_t end = std::get<1>(domain);
    while (i < start) {
      emit_non_applicable_rule(rules_[i], out);
      ++i;
    }
    emit_tree(trees_[j], out);
    i = end + 1;
  }
  for (; i < num_rules; ++i)
    emit_non_applicable_rule(rules_[i], out);
  emit_suffix(out);
}


void Emitter::emit_tree(TreeNode& tree, std::stringstream& out) {
  if (search_ == Arguments::SEARCH_LINEAR)
    emit_tree_linear_search(tree, "", out);
  ///else
  ///  emit_tree_binary_search(tree, out);
}


void Emitter::emit_tree_linear_search(TreeNode& tree,
    const std::string& next_chain, std::stringstream& out) {

  const std::string& tree_chain = tree.chain();
  std::stringstream ss;
  size_t chain_count = 0;
  ss << tree_chain << "_" << chain_count;
  chain_count++;
  std::string start_chain(ss.str());
  ss.str("");
  out << "# Chain " << tree_chain << std::endl;
  out << "-A " << tree_chain << " -j " << start_chain << std::endl;
  NodeRefQueue node_fifo;
  std::queue<std::string> chain_fifo;
  node_fifo.push(&tree);
  chain_fifo.push(start_chain);

  while (!node_fifo.empty()) {
    TreeNode* current_node = node_fifo.front();
    node_fifo.pop();
    std::string& chain = chain_fifo.front();
    chain_fifo.pop();
    emit_simple_linear_dispatch(current_node, chain, chain_count, out);
  }
  out << std::endl;
}


void Emitter::emit_leaf(const TreeNode* node, const std::string& current_chain,
    const std::string& next_chain, std::stringstream& out) {
  
  out << "# leaf node" << std::endl;
  const size_t num_rules = node->num_rules();
  for (size_t i = 0; i < num_rules; ++i) {
    out << node->rules()[i]->src_with_patched_chain(current_chain)
        << std::endl;
  }
  out << "-A " << current_chain << " -j " << next_chain << std::endl
      << std::endl;
}


void Emitter::emit_simple_linear_dispatch(const TreeNode* node,
    const std::string& chain, const size_t chain_count,
    std::stringstream& out) {

  
}

///void Emitter::emit_tree_binary_search(const TreeNode& tree,
///    std::stringstream& out) {
///
///  
///}


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
