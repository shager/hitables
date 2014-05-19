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


void Emitter::emit_tree(const TreeNode& tree, std::stringstream& out) {

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
