#ifndef HITABLES_PARTIAL_RULESET_HPP
#define HITABLES_PARTIAL_RULESET_HPP 1

#include "rule.hpp"

class PartialRuleset {
public:
  PartialRuleset(const size_t start, const size_t end,
      const std::vector<Rule>& rules)
      : start_(start), end_(end), rules_(rules) {}

  inline size_t start() const {return start_;}
  inline size_t end() const {return end_;}
  inline const std::vector<Rule>& rules() const {return rules_;}

private:
  const size_t start_;
  const size_t end_;
  const std::vector<Rule> rules_;
};

typedef std::vector<PartialRuleset> RulesetVector;

#endif // HITABLES_PARTIAL_RULESET_HPP
