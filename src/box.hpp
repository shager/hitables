#ifndef HITABLES_BOX_HPP
#define HITABLES_BOX_HPP 1

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <tuple>

typedef uint32_t dim_t;
typedef std::tuple<dim_t, dim_t> DimTuple;
typedef std::vector<DimTuple> DimVector;

class Box {

public:
  Box(const DimVector& box_bounds)
      : box_bounds_(box_bounds), num_dims_(box_bounds.size()) {}

  bool operator==(const Box& other) const;
  inline bool operator!=(const Box& other) const {return !(*this == other);}

  void cut(const size_t dimension, const size_t num_cuts,
      std::vector<Box>& result_boxes) const;

  bool collide(const Box& other) const;

  static size_t num_distinct_boxes_in_dim(const size_t dimension,
      const std::vector<const Box*>& rules);

  inline const DimVector& box_bounds() const {return box_bounds_;}
  inline size_t num_dims() const {return num_dims_;}

private:
  DimVector box_bounds_;
  size_t num_dims_;
};

#endif // HITABLES_BOX_HPP
