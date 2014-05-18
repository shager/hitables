#include "action.hpp"

Action& Action::operator=(const Action& rhs) {
  code_ = rhs.code();
  next_chain_ = rhs.next_chain();
  return *this;
}

bool Action::operator==(const Action& other) const {
  return ((code_ == other.code()) && (next_chain_ == other.next_chain()));
}
