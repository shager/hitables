#include "rule.hpp"

size_t Rule::num_distinct_rules_in_dim(const size_t dim,
    const std::vector<const Rule*>& rules) {

  const size_t num_rules = rules.size();
  size_t num_distinct = 0;
  for (size_t i = 0; i < num_rules; ++i) {
    const DimTuple& interval1 = rules[i]->box().box_bounds()[dim];
    const dim_t start1 = std::get<0>(interval1);
    const dim_t end1 = std::get<1>(interval1);
    // now scan through each other rule and check for collisions
    size_t distinct = 1;
    for (size_t j = 0; j < num_rules; ++j) {
      if (i == j)
        continue;
      const DimTuple& interval2 = rules[j]->box().box_bounds()[dim];
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


bool Rule::operator==(const Rule& other) const {
  return ((box_ == other.box())
      &&  (action_ == other.action())
      &&  (applicable_ == other.applicable())
      &&  (chain_ == other.chain()));
}

std::string Rule::src_with_patched_chain(const std::string& chain) const {
  const size_t start = src_.find(chain_);
  const size_t end = src_.find(" ", start + 2);
  std::stringstream ss;
  ss << src_.substr(0, start);
  ss << chain << " ";
  ss << src_.substr(end + 1);
  return ss.str();
}


void Rule::delete_rules(RuleVector& rules) {
  const size_t num_rules = rules.size();
  for (size_t i = 0; i < num_rules; ++i)
    delete rules[i];
}
