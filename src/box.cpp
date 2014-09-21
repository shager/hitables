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


void Box::unequal_cut(const size_t dimension,
    const std::vector<dim_t>& cut_points,
    std::vector<Box>& result_boxes) const {

  dim_t start = std::get<0>(box_bounds_[dimension]);
  dim_t end = 0;
  const dim_t box_end = std::get<1>(box_bounds_[dimension]);
  const size_t num_cut_points = cut_points.size();
  for (size_t i = 0; i < num_cut_points; ++i) {
    end = cut_points[i];
    DimVector box_bounds(box_bounds_);
    box_bounds[dimension] = std::make_tuple(start, end);
    result_boxes.push_back(Box(box_bounds));
    start = end + 1;
  }
  if (end < box_end) {
    DimVector box_bounds(box_bounds_);
    box_bounds[dimension] = std::make_tuple(start, box_end);
    result_boxes.push_back(Box(box_bounds));
  }
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
