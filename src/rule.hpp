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
      const std::vector<const Rule*>& rules);

  bool operator==(const Rule& other) const;

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
