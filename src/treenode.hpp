#ifndef HITABLES_TREENODE_HPP
#define HITABLES_TREENODE_HPP 1

#include <cstdlib>
#include <vector>
#include <queue>
#include <cmath>
#include "box.hpp"

class TreeNode {

public:
  TreeNode(const DimVector& bounds)
      : box_(bounds), has_been_cut_(false) {}

  TreeNode(const Box& box) : box_(box), has_been_cut_(false) {}

  /*
   * Cuts this node along the given dimension the specified number of times.
   * The new resulting nodes are then stored in the children_ vector.
   */
  void cut(const dim_t dimension, const size_t num_cuts);
  
  /*
   * Computes the space measure functionality as defined in the HiCuts paper.
   */
  size_t space_measure() const;

  /*
   * Reset the cut of the tree node in order to make it cuttable again.
   */
  inline void reset_cut() {
    children_.clear();
    has_been_cut_ = false;
  }

  /*
   * Computes the upper bound for the space measure function as defined in the
   * HiCuts paper.
   */
  inline size_t space_measure_upper_bound(const size_t spfac) const {
    return spfac * rules_.size();
  }

  /*
   * Determines the number of cuts to perform along the specified dimension and
   * with regard to the spfac parameter.
   * Although this is not a const method, it reverts all changes that are being
   * done to the tree node object during its execution.
   */
  size_t determine_number_of_cuts(const size_t dimension, const size_t spfac);

  /*
   * Detects the dimension in which the tree node's rules differ the most.
   * If there is no such dimension, it randomly picks one of the highest
   * differing dimensions.
   */
  size_t dim_max_distinct_rules() const;

  /*
   * Finds the dimension that exhibits with the least number of rules in the
   * biggest child after a cutting process.
   * If there are several candidates, one of them is randomly picked.
   * Although this is not a const method, it reverts all changes that are being
   * done to the tree node object during its execution.
   */
  size_t dim_least_max_rules_per_child(const size_t spfac);

  /*
   * Selects a random dimension for cutting.
   */
  inline size_t random_dim() const {return rand() % box_.num_dims();}

  /*
   * Builds a HiCuts tree with this node as tree root.
   * spfac and binth are parameters that influence the shape of the constructed
   * tree, as described in the paper.
   */
  void build_tree(const size_t spfac, const size_t binth);

  inline size_t num_rules() const {return rules_.size();}
  inline void add_rule(const Box* rule) {rules_.push_back(rule);}
  inline const Box& box() const {return box_;}
  inline std::vector<TreeNode>& children() {return children_;}
  inline const std::vector<const Box*> rules() const {return rules_;}

private:
  Box box_;
  std::vector<const Box*> rules_;
  std::vector<TreeNode> children_;
  bool has_been_cut_;

  inline size_t max(const size_t a, const size_t b) const {
    return a > b ? a : b;
  }

  inline size_t min(const size_t a, const size_t b) const {
    return a < b ? a : b;
  }
};

typedef std::queue<TreeNode*> NodeRefQueue;
typedef std::vector<TreeNode> NodeVector;

#endif // HITABLES_TREENODE_HPP
