#ifndef HITABLES_RULE_HPP
#define HITABLES_RULE_HPP 1

#include "box.hpp"
#include "action.hpp"
  
const dim_t ICMP = 1;
const dim_t TCP = 6;
const dim_t UDP = 17;
const dim_t PROTOCOL_WILDCARD = 256;

const dim_t min_port = 0;
const dim_t max_port = 65535;

const dim_t min_ip = 0;
const dim_t max_ip = UINT32_MAX;

const dim_t min_prot = 0;
const dim_t max_prot = 255;

class Rule {
public:
  Rule(const Action& action, const Box& box) : action_(action), box_(box),
      applicable_(true), chain_("") {}

  Rule(const Action& action, const DimVector& dims, const std::string& chain)
      : action_(action), box_(dims), applicable_(true), chain_(chain) {}

  Rule() : action_(Action(NONE)), box_(DimVector()), applicable_(false) {}

  inline const Action& action() const {return action_;}

  inline const Box& box() const {return box_;}

  inline bool applicable() const {return applicable_;}

  inline const std::string& chain() const {return chain_;}

  static size_t num_distinct_rules_in_dim(const size_t dim,
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

  bool operator==(const Rule& other) const {
    return ((box_ == other.box())
        &&  (action_ == other.action())
        &&  (applicable_ == other.applicable())
        &&  (chain_ == other.chain()));
  }

  inline bool operator!=(const Rule& other) const {return !(*this == other);}

private:
  const Action action_;
  Box box_;
  const bool applicable_;
  std::string chain_;
};

typedef std::vector<Rule> RuleVector;
typedef std::vector<std::tuple<size_t, size_t>> DomainVector;

#endif // HITABLES_RULE_HPP
