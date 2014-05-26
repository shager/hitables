#include "rule.hpp"

size_t Rule::num_distinct_rules_in_dim(const size_t dim,
    std::vector<const Rule*>& rules) {

  const size_t num_rules = rules.size();
  if (num_rules <= 1)
    return num_rules;

  // sort rules by the given dimension
  std::sort(rules.begin(), rules.end(), [dim] (const Rule* a, const Rule* b) {
    return a->box().box_bounds()[dim] <= b->box().box_bounds()[dim];
  });

  // consider rules pairwise
  size_t num_distinct = 0;
  const size_t loop_end = num_rules - 1;
  // check first rule
  const dim_t current_end = std::get<1>(rules[0]->box().box_bounds()[dim]);
  const dim_t next_start = std::get<0>(rules[1]->box().box_bounds()[dim]);
  if (current_end < next_start)
    ++num_distinct;
  // check rules 1 to n - 2
  dim_t highest_end = current_end;
  for (size_t i = 1; i < loop_end; ++i) {
    const Box& box = rules[i]->box();
    const dim_t current_start = std::get<0>(box.box_bounds()[dim]);
    const dim_t current_end = std::get<1>(box.box_bounds()[dim]);
    const dim_t next_start = std::get<0>(
        rules[i + 1]->box().box_bounds()[dim]);
    if (current_start > highest_end && current_end < next_start)
      ++num_distinct;
    highest_end = highest_end < current_end ? current_end : highest_end;
  }
  // check last rule
  const dim_t current_start = std::get<0>(
      rules[loop_end]->box().box_bounds()[dim]);
  if (current_start > highest_end)
    ++num_distinct;
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
