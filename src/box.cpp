#include "box.hpp"

bool Box::operator==(const Box& other) const {
  if (num_dims_ != other.num_dims())
    return false;
  const DimVector& other_bounds = other.box_bounds();
  for (size_t i = 0; i < num_dims_; ++i) {
    const DimTuple& my_interval = box_bounds_[i];
    const DimTuple& other_interval = other_bounds[i];
    const dim_t my_start = std::get<0>(my_interval);
    const dim_t my_end = std::get<1>(my_interval);
    const dim_t other_start = std::get<0>(other_interval);
    const dim_t other_end = std::get<1>(other_interval);
    if (my_start != other_start || my_end != other_end)
      return false;
  }
  return true;
}


void Box::cut(const size_t dimension, const size_t num_cuts,
    std::vector<Box>& result_boxes) const {

  const DimTuple& bounds = box_bounds_[dimension];
  const dim_t start = std::get<0>(bounds);
  const dim_t end = std::get<1>(bounds);
  const size_t piece_len = (end - start) / (num_cuts + 1);

  dim_t current_start = start;
  for (size_t i = 0; i < num_cuts; ++i) {
    const dim_t current_end = current_start + piece_len;
    DimVector box_bounds(box_bounds_);
    box_bounds[dimension] = std::make_tuple(current_start, current_end);
    result_boxes.push_back(Box(box_bounds));
    current_start = current_end + 1;
  }
  DimVector box_bounds(box_bounds_);
  box_bounds[dimension] = std::make_tuple(current_start, end);
  result_boxes.push_back(Box(box_bounds));
}


bool Box::collide(const Box& other) const {
  const DimVector& other_bounds = other.box_bounds();
  // two boxes do not collide if they are disjunct in at least one dimension
  for (size_t dim = 0; dim < num_dims_; ++dim) {
    const DimTuple& my_interval = box_bounds_[dim];
    const DimTuple& other_interval = other_bounds[dim];
    const dim_t my_start = std::get<0>(my_interval);
    const dim_t my_end = std::get<1>(my_interval);
    const dim_t other_start = std::get<0>(other_interval);
    const dim_t other_end = std::get<1>(other_interval);
    if (my_start > other_end || my_end < other_start)
      return false;
  }
  return true;
}


size_t Box::num_distinct_boxes_in_dim(const size_t dim,
    const std::vector<const Box*>& rules) {
  
  const size_t num_rules = rules.size();
  size_t num_distinct = 0;
  for (size_t i = 0; i < num_rules; ++i) {
    const DimTuple& interval1 = rules[i]->box_bounds()[dim];
    const dim_t start1 = std::get<0>(interval1);
    const dim_t end1 = std::get<1>(interval1);
    // now scan through each other rule and check for collisions
    size_t distinct = 1;
    for (size_t j = 0; j < num_rules; ++j) {
      if (i == j)
        continue;
      const DimTuple& interval2 = rules[j]->box_bounds()[dim];
      const dim_t start2 = std::get<0>(interval2);
      const dim_t end2 = std::get<1>(interval2);
      if (!(start1 > end2 || end1 < start2)) {
        distinct = 0;
        break;
      }
    }
    num_distinct += distinct;
  }
  return num_distinct;
}
