#ifndef EMIT_HPP
#define EMIT_HPP 1

#include <cstdlib>
#include "treenode.hpp"

class Emitter {
public:

  Emitter(const NodeVector& trees, const RuleVector& rules,
      const DomainVector& domains, const bool chain_reuse, const size_t search)
      : trees_(trees), rules_(rules), domains_(domains),
      chain_reuse_(chain_reuse), search_(search) {}

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

  void emit_tree(const TreeNode& tree, std::stringstream& out);

private:
  NodeVector trees_;
  RuleVector rules_;
  DomainVector domains_;
  const bool chain_reuse_;
  const size_t search_;
};

#endif // EMIT_HPP
