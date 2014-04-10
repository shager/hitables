#ifndef HITABLES_RULE_HPP
#define HITABLES_RULE_HPP 1

#include "box.hpp"

enum Action {DROP, REJECT, ACCEPT};

class Rule {
public:
  Rule(const Action action, const Box& box) : action_(action), box_(box) {}
  inline Action action() const {return action_;}
  inline const Box& box() const {return box_;}

private:
  const Action action_;
  Box box_;
};

typedef std::vector<Rule> RuleVector;

#endif // HITABLES_RULE_HPP
