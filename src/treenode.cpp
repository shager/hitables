#include "treenode.hpp"

/*
 * Takes the boxes resulting from a local cut and the rules in a tree node and
 * builds up the children vector.
 */
static void build_children(const std::vector<Box>& result_boxes,
    const std::vector<const Rule*>& rules,
    std::vector<TreeNode>& children) {

  const size_t num_result_boxes = result_boxes.size();
  const size_t num_rules = rules.size();
  for (size_t i = 0; i < num_result_boxes; ++i) {
    const Box& node_box = result_boxes[i];
    TreeNode node(node_box);
    for (size_t j = 0; j < num_rules; ++j) {
      if (rules[j]->box().collide(node_box))
        node.add_rule(rules[j]);
    }
    // add this node to the children if it is not empty
    if (node.num_rules() > 0)
      children.push_back(node);
  }
}


void TreeNode::cut(const dim_t dimension, const size_t num_cuts) {
  if (has_been_cut_)
    return;
  // perform the cut
  std::vector<Box> result_boxes;
  box_.cut(dimension, num_cuts, result_boxes);
  build_children(result_boxes, rules_, children_);
  // add meta information
  has_been_cut_ = true;
  cut_dim_ = dimension;
  num_cuts_ = num_cuts;
}


void TreeNode::unequal_cut(const dim_t dimension,
    const std::vector<dim_t>& cut_points) {

  if (has_been_cut_)
    return;
  ///std::vector<const Rule*> rules_copy(rules_);
  ///std::vector<dim_t> cut_points;
  ///Rule::cut_points(dimension, rules_copy, cut_points);
  const size_t num_cut_points = cut_points.size();
  // check if an unequal cut can be performed at at least two cut points
  if (num_cut_points <= 1)
    return;
  // perform the cut
  std::vector<Box> result_boxes;
  box_.unequal_cut(dimension, cut_points, result_boxes);
  build_children(result_boxes, rules_, children_);
  // add meta information
  has_been_cut_ = true;
  cut_dim_ = dimension;
  num_cuts_ = num_cut_points;
}


size_t TreeNode::space_measure() const {
  size_t space_measure = 0;
  const size_t num_children = children_.size();
  for (size_t i = 0; i < num_children; ++i)
    space_measure += children_[i].rules().size();
  space_measure += (num_cuts_ == 0 ? 0 : num_cuts_ + 1);
  return space_measure;
}


size_t TreeNode::determine_number_of_cuts(const size_t dimension,
    const size_t spfac) {

  const size_t num_rules = rules_.size();
  const size_t square_root = sqrt(num_rules);
  size_t num_cuts = max(4, square_root);
  for (;;) {
    cut(dimension, num_cuts);
    const size_t space = space_measure();
    const size_t threshold = space_measure_upper_bound(spfac);
    reset_cut();
    if (space < threshold)
      num_cuts <<= 1;
    else
      break;
  }
  const DimTuple& interval = box_.box_bounds()[dimension];
  //const dim_t max_cuts = std::get<1>(interval) - std::get<0>(interval) - 1;
  const dim_t max_cuts = std::get<1>(interval) - std::get<0>(interval);
  return min(num_cuts, max_cuts);
}


#include <iostream>
#include <chrono>
#include <thread>

std::tuple<size_t, bool> TreeNode::dim_max_distinct_rules() const {
  const size_t num_dims = box_.num_dims();
  size_t* distinct_rules = new size_t[num_dims];
  size_t max_distinct = 0;
  std::vector<const Rule*> rules_copy(rules_);
  for (size_t i = 0; i < num_dims; ++i) {
    const size_t num_distinct = Rule::num_distinct_rules_in_dim(i, rules_copy);
    max_distinct = max_distinct < num_distinct ? num_distinct : max_distinct;
    distinct_rules[i] = num_distinct;
  }
  // gather all dimensions with the highest number of distinct rules
  ////std::cout << "max distinct = " << max_distinct << std::endl;
  ////std::cout << "num rules = " << rules_.size() << std::endl;
  std::vector<size_t> max_dims;
  dim_t max_dim_size = 0;
  const DimVector& bounds = box_.box_bounds();
  for (size_t i = 0; i < num_dims; ++i)
    if (distinct_rules[i] == max_distinct) {
      max_dims.push_back(i);
      const dim_t dim_size = std::get<1>(bounds[i]) - std::get<0>(bounds[i]);
      max_dim_size = dim_size > max_dim_size ? dim_size : max_dim_size;
    }
  delete[] distinct_rules;
  // compute those dimensions with the largest span
  std::vector<size_t> max_span_dims;
  const size_t num_max_dims = max_dims.size();
  for (size_t i = 0; i < num_max_dims; ++i) {
    const size_t dim = max_dims[i];
    const dim_t dim_size = std::get<1>(bounds[dim]) - std::get<0>(bounds[dim]);
    if (dim_size == max_dim_size)
      max_span_dims.push_back(dim);
  }
  // randomly select one of them
  ////const size_t cut_dim = max_dims[rand() % max_dims.size()];
  const size_t cut_dim = max_span_dims[rand() % max_span_dims.size()];
  ///std::cout << "cut dim = " << cut_dim << std::endl;
  //////return cut_dim;
  const bool have_distinct_dim = max_distinct > 0;
  return std::make_tuple(cut_dim, have_distinct_dim);
}


size_t TreeNode::dim_least_max_rules_per_child(const size_t spfac) {
  const size_t num_dims = box_.num_dims();
  size_t* least_max_nodes = new size_t[num_dims];
  size_t least_max = rules_.size() + 1;
  for (size_t i = 0; i < num_dims; ++i) {
    const size_t num_cuts = determine_number_of_cuts(i, spfac);
    cut(i, num_cuts);
    // find the child that contains most rules
    size_t max_rules = 0;
    const size_t num_children = children_.size();
    for (size_t j = 0; j < num_children; ++j) {
      const size_t num_rules = children_[j].rules().size();
      max_rules = max_rules < num_rules ? num_rules : max_rules;
    }
    reset_cut();
    least_max_nodes[i] = max_rules;
    least_max = least_max > max_rules ? max_rules : least_max;
  }
  // gather the dimension with the least number of rules in the biggest nodes
  std::vector<size_t> dims;
  for (size_t i = 0; i < num_dims; ++i)
    if (least_max_nodes[i] == least_max)
      dims.push_back(i);
  delete[] least_max_nodes;
  return dims[rand() % dims.size()];
}


size_t TreeNode::dim_most_distinct_projection_points(
    std::vector<dim_t>& points) const {

  const size_t num_dims = box_.num_dims();
  const size_t num_rules = rules_.size();
  std::vector<std::set<dim_t>> point_sets;
  size_t max_points = 0;
  for (size_t i = 0; i < num_dims; ++i) {
    const dim_t box_start = std::get<0>(box_.box_bounds()[i]);
    const dim_t box_end = std::get<1>(box_.box_bounds()[i]);
    std::set<dim_t> points;
    for (size_t j = 0; j < num_rules; ++j) {
      const dim_t start = std::get<0>(rules_[j]->box().box_bounds()[i]);
      const dim_t end = std::get<1>(rules_[j]->box().box_bounds()[i]);
      if (start >= box_start)
        points.insert(start);
      if (end <= box_end)
        points.insert(end);
    }
    const size_t num_points = points.size();
    max_points = num_points > max_points ? num_points : max_points;
    point_sets.push_back(points);
  }
  // find those dimensions with the maximum number of distinct points
  std::vector<size_t> max_dims;
  for (size_t i = 0; i < num_dims; ++i)
    if (point_sets[i].size() == max_points)
      max_dims.push_back(i);
  // randomly select one of these
  const size_t max_dim = max_dims[rand() % max_dims.size()];
  std::set<dim_t>& target_set = point_sets[max_dim];
  for (auto it = target_set.begin(); it != target_set.end(); ++it)
    points.push_back(*it);
  return max_dim;
}


void TreeNode::build_tree(const size_t spfac, const size_t binth,
    const size_t dim_choice, const size_t cut_algo) {

  NodeRefQueue fifo;
  fifo.push(this);
  size_t cut_dim = 0;
  while (!fifo.empty()) {
    TreeNode* node = fifo.front();
    fifo.pop();
    if (node->num_rules() <= binth)
      continue;
    // perform the cut
    if (cut_algo == Arguments::CUT_ALGO_EQUIDISTANT) {
      // equidistant cut
      if (dim_choice == Arguments::DIM_CHOICE_LEAST_MAX_RULES)
        cut_dim = node->dim_least_max_rules_per_child(spfac);
      else {
        std::tuple<size_t, bool> distinct(node->dim_max_distinct_rules());
        const bool have_distinct_dim = std::get<1>(distinct);
        if (have_distinct_dim)
          // if we have a dimension with distinct rules, take it
          cut_dim = std::get<0>(distinct);
        else {
          // otherwise, try to find the dimension with most distinct projection
          // points
          std::vector<dim_t> unused;
          cut_dim = node->dim_most_distinct_projection_points(unused);
        }
        ////////std::cout << "have distinct dim = " << have_distinct_dim << std::endl;
        ////////std::cout << "cut dim = " << cut_dim << std::endl;
      }
      const size_t num_cuts = node->determine_number_of_cuts(cut_dim, spfac);
      node->cut(cut_dim, num_cuts);
    } else {
      // unequal cut
      std::tuple<size_t, bool> distinct(node->dim_max_distinct_rules());
      cut_dim = std::get<0>(distinct);
      ///////////cut_dim = node->dim_max_distinct_rules();
      ////std::cout << "cut dim (distinct): " << cut_dim << std::endl;
      std::vector<dim_t> cut_points;
      std::vector<const Rule*> rules_copy(node->rules());
      ////std::cout << "num cut points before computation: " << cut_points.size() << std::endl;
      Rule::cut_points(cut_dim, rules_copy, cut_points);
      ////std::cout << "cut points = ";
      ////for (size_t i = 0; i < cut_points.size(); ++i)
      ////  std::cout << cut_points[i] << " ";
      ////std::cout << std::endl;
      node->unequal_cut(cut_dim, cut_points);
      ////std::cout << "cut dim (distinct) = " << cut_dim << std::endl;
      ////std::cout << "has been cut (distinct) = " << node->has_been_cut() << std::endl;
      // check whether we have to cut on projection points (if there were no
      // distinct rules)
      if (!node->has_been_cut()) {
        std::vector<dim_t> projection_points;
        cut_dim = node->dim_most_distinct_projection_points(projection_points);
        ////std::cout << "num proj. points = " << projection_points.size() << std::endl;
        ////std::cout << "cut dim (proj.) = " << cut_dim << std::endl;
        node->unequal_cut(cut_dim, projection_points);
        ////std::cout << "has been cut (proj.) = " << node->has_been_cut() << std::endl;
        ////std::cout << "Proj. points = ";
        ////for (auto it = projection_points.begin(); it != projection_points.end(); ++it)
        ////  std::cout << *it << " ";
        ////std::cout << std::endl;
        // if the node still has not been cut yet, perform an equidistant cut
        if (!node->has_been_cut()) {
          const size_t num_cuts = node->determine_number_of_cuts(cut_dim, spfac);
          ////std::cout << "num cuts = " << num_cuts << std::endl;
          node->cut(cut_dim, num_cuts);
        }
      }
    }
    // add children to the tree if they are large enough
    NodeVector& children = node->children();
    const size_t num_children = children.size();
    //std::cout << "num children = " << num_children << std::endl;

    ///bool all_same_size = true;
    ///for (size_t i = 0; i < num_children; ++i) {
    ///  if (children[i].num_rules() != node->num_rules()) {
    ///    all_same_size = false;
    ///    break;
    ///  }
    ///}
    ///std::cout << "all same size = " << all_same_size << std::endl;
    ///if (all_same_size) {
    ///////////std::cout << "Node: [" << std::get<0>(node->box().box_bounds()[cut_dim])
    ///////////    << ", " << std::get<1>(node->box().box_bounds()[cut_dim]) << "]"
    ///////////    << std::endl;
    ///////////std::cout << "Rules: ";
    ///////////const std::vector<const Rule*>& rules = node->rules();
    ///////////const size_t num_dims = node->box().box_bounds().size();
    ///////////for (size_t j = 0; j < num_dims; ++j) {
    ///////////  std::cout << "- ";
    ///////////  if (j == cut_dim)
    ///////////    std::cout << "CUT DIM";
    ///////////  std::cout << " -\n";
    ///////////  for (size_t i = 0; i < rules.size(); ++i) {
    ///////////    std::cout << "[" << std::get<0>(rules[i]->box().box_bounds()[j])
    ///////////        << ", " << std::get<1>(rules[i]->box().box_bounds()[j])
    ///////////        << "] ";
    ///////////  }
    ///////////  std::cout << std::endl;
    ///////////}
    ///////////std::cout << "Children spans in cut dim: ";
    ///////////for (size_t i = 0; i < num_children; ++i) {
    ///////////  std::cout << "[" << std::get<0>(children[i].box().box_bounds()[cut_dim])
    ///////////      << ", " << std::get<1>(children[i].box().box_bounds()[cut_dim])
    ///////////      << "] ";
    ///////////}
    ///////////std::cout << std::endl;
    ///}

    //////std::cout << "Child rule sizes:" << std::endl;
    for (size_t i = 0; i < num_children; ++i) {
      TreeNode& child = children[i];
      ///////std::cout << child.rules().size() << " ";
      if (child.num_rules() > binth)
        fifo.push(&child);
    }
    //////std::cout << std::endl << "fifo size = " << fifo.size() << std::endl;
    //////std::cout << "----------------------" <<  std::endl;
    //////std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}


Box TreeNode::minimal_bounding_box(const RuleVector& rules,
    const DomainTuple& domain) {

  const size_t num_rules = rules.size();
  DimVector dims;
  if (num_rules == 0)
    return Box(dims);
  const size_t num_dims = rules[0]->box().num_dims();
  for (size_t i = 0; i < num_dims; ++i) {
    dim_t min = max_ip;
    dim_t max = min_ip;
    const size_t start = std::get<0>(domain);
    const size_t end = std::get<1>(domain);
    for (size_t j = start; j <= end; ++j) {
      const DimVector& dims = rules[j]->box().box_bounds();
      const DimTuple& dim_tuple = dims[i];
      min = min < std::get<0>(dim_tuple) ? min : std::get<0>(dim_tuple);
      max = max > std::get<1>(dim_tuple) ? max : std::get<1>(dim_tuple);
    }
    dims.push_back(std::make_tuple(min, max));
  }
  return Box(dims);
}


Box TreeNode::minimal_bounding_box(const NodeVector& nodes,
    const DomainTuple& domain) {

  const size_t num_nodes = nodes.size();
  DimVector dims;
  if (num_nodes == 0)
    return Box(dims);
  const size_t num_dims = nodes[0].box().num_dims();
  for (size_t i = 0; i < num_dims; ++i) {
    dim_t min = max_ip;
    dim_t max = min_ip;
    const size_t start = std::get<0>(domain);
    const size_t end = std::get<1>(domain);
    for (size_t j = start; j <= end; ++j) {
      const DimVector& dims = nodes[j].box().box_bounds();
      const DimTuple& dim_tuple = dims[i];
      min = min < std::get<0>(dim_tuple) ? min : std::get<0>(dim_tuple);
      max = max > std::get<1>(dim_tuple) ? max : std::get<1>(dim_tuple);
    }
    dims.push_back(std::make_tuple(min, max));
  }
  return Box(dims);
}


std::string TreeNode::prot() const {
  return rules_[0]->protocol() == TCP ? "tcp" : "udp";
}


void TreeNode::compute_numbering() {
  NodeRefStack node_stack;
  node_stack.push(this);
  size_t current_id = 0;
  while (!node_stack.empty()) {
    TreeNode* current_node = node_stack.top();
    node_stack.pop();
    current_node->set_id(current_id++);
    NodeVector& children = current_node->children();
    const size_t num_children = children.size();
    for (int i = num_children - 1; i >= 0; --i)
      node_stack.push(&children[i]);
  }
}


void TreeNode::add_rule(const Rule* rule) {
  const size_t num_rules_ = num_rules();
  for (size_t i = 0; i < num_rules_; ++i)
    if (rule->is_shadowed(rules_[i], box_))
      return;
  rules_.push_back(rule);
}
