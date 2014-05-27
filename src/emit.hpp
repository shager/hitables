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

  void emit_tree(TreeNode* tree,
      const std::string& chain, const size_t tree_id,
      const std::string& next_chain, const bool leaf_jump,
      std::stringstream& out, StrVector& chains);

  void emit_simple_linear_dispatch(TreeNode* node,
      const std::string& chain, const size_t tree_id,
      const size_t chain_count, std::stringstream& out, StrVector& chains);

  void emit_simple_binary_dispatch(TreeNode* node,
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


class BinSearchTree {
public:

  BinSearchTree(const size_t start, const size_t end) : start_(start),
      end_(end), lookup_index_(((end - start) >> 1) + start), left_(nullptr),
      right_(nullptr) {
  
    const size_t diff = end - start;
    if (diff == 0)
      return;
    if (lookup_index_ > start)
      left_ = new BinSearchTree(start,
          lookup_index_ == 0 ? 0 : lookup_index_ - 1);
    right_ = new BinSearchTree(lookup_index_ + 1, end);
  }

  ~BinSearchTree() {
    if (has_left_child())
      delete left_;
    if (has_right_child())
      delete right_;
  }

  inline bool is_leaf() const {return left_ == 0 && right_ == 0;}

  inline size_t start() const {return start_;}

  inline size_t end() const {return end_;}

  inline bool has_left_child() const {return left_ != nullptr;}

  inline const BinSearchTree* left() const {return left_;}

  inline bool has_right_child() const {return right_ != nullptr;}

  inline const BinSearchTree* right() const {return right_;}

  inline size_t lookup_index() const {return lookup_index_;}

private:
  size_t start_;
  size_t end_;
  size_t lookup_index_;
  BinSearchTree* left_;
  BinSearchTree* right_;
};

#endif // EMIT_HPP
