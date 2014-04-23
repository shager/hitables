#ifndef HITABLES_ACTION_HPP
#define HITABLES_ACTION_HPP 1

#include <string>

enum ActionCode {DROP, REJECT, ACCEPT, JUMP, NONE};

class Action {
public:
  Action(const ActionCode code) : code_(code), next_chain_("") {}
  Action(const ActionCode code, const std::string& next_chain)
      : code_(code), next_chain_(next_chain) {}

  inline ActionCode code() const {return code_;}
  inline const std::string& next_chain() const {return next_chain_;}

private:
  const ActionCode code_;
  const std::string next_chain_;
};

#endif // HITABLES_ACTION_HPP
