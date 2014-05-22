#ifndef EMIT_HPP
#define EMIT_HPP 1

#include <cstdlib>
#include <cstdio>
#include "treenode.hpp"

class Emitter {
public:

  Emitter(const NodeVector& trees, const RuleVector& rules,
      const DomainVector& domains, const size_t search)
      : trees_(trees), rules_(rules), domains_(domains),
      search_(search) {}

  /*
   * Computes the iptables representation of the given HiTables instance and
   * writes it to the specified out stream.
   */
  void emit(std::stringstream& out);

  void emit_prefix(std::stringstream& out);

  void emit_suffix(std::stringstream& out);

  inline void emit_non_applicable_rule(const Rule& rule,
      std::stringstream& out) {

    out << rule.src() << std::endl;
  }

  void emit_tree(TreeNode& tree, std::stringstream& out);

  void emit_tree_linear_search(TreeNode& tree,
      const std::string& next_chain, std::stringstream& out);

  void emit_simple_linear_dispatch(const TreeNode* node,
      const std::string& chain,
      const size_t chain_count, std::stringstream& out);

  void emit_leaf(const TreeNode* node, const std::string& current_chain,
      const std::string& next_chain, std::stringstream& out);

  static std::string num_to_ip(const dim_t ip_num);

private:
  NodeVector trees_;
  RuleVector rules_;
  DomainVector domains_;
  const size_t search_;
};

#endif // EMIT_HPP
