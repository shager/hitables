#include "action.hpp"

Action& Action::operator=(const Action& rhs) {
  code_ = rhs.code();
  next_chain_ = rhs.next_chain();
  return *this;
}
