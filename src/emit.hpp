#ifndef EMIT_HPP
#define EMIT_HPP 1

#include <cstdlib>
#include <cstdio>
#include "treenode.hpp"
//#include "parse.hpp"

class Emitter {
public:

  Emitter(const NodeRefVector& trees, const RuleVector& rules,
      const DomainVector& domains, const size_t search)
      : trees_(trees), rules_(rules), domains_(domains),
      search_(search) {}

  /*
   * Computes the iptables representation of the given HiTables instance and
   * writes it to the specified out stream.
   */
  void emit(std::stringstream& out, StrVector& chains);

  static void emit_prefix(std::stringstream& out);

  static void emit_suffix(std::stringstream& out);

  void emit_non_applicable_rule(const Rule* rule, const std::string& chain,
      std::stringstream& out);

  void emit_tree(TreeNode* tree, const std::string& chain,
      const size_t tree_id, const std::string& next_chain,
      const bool leaf_jump, std::stringstream& out, StrVector& chains);

  void emit_tree_linear_search(TreeNode* tree, const std::string& chain,
      const size_t tree_id, const std::string& next_chain,
      const bool leaf_jump, std::stringstream& out, StrVector& chains);

  void emit_simple_linear_dispatch(TreeNode* node,
      const std::string& chain, const size_t tree_id,
      const size_t chain_count, std::stringstream& out, StrVector& chains);

  void emit_leaf(const TreeNode* node, const std::string& current_chain,
      const std::string& next_chain, const bool leaf_jump,
      std::stringstream& out);

  static std::string num_to_ip(const dim_t ip_num);

private:
  NodeRefVector trees_;
  RuleVector rules_;
  DomainVector domains_;
  const size_t search_;
};

#endif // EMIT_HPP
