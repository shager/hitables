#ifndef HITABLES_RULE_HPP
#define HITABLES_RULE_HPP 1

#include "box.hpp"
#include "action.hpp"
#include <sstream>
#include <algorithm>
  
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

class Rule;

typedef std::vector<Rule*> RuleVector;
typedef std::vector<RuleVector> ChainVector;
typedef std::tuple<size_t, size_t> DomainTuple;
typedef std::vector<DomainTuple> DomainVector;

class Rule {
public:
  Rule(const Action& action, const Box& box, const std::string& src) 
      : action_(action), box_(box), applicable_(true), chain_(""), src_(src),
      protocol_(PROTOCOL_WILDCARD) {}

  Rule(const Action& action, const DimVector& dims, const std::string& chain,
      const std::string& src, const size_t protocol)
      : action_(action), box_(dims), applicable_(true), chain_(chain),
      src_(src), protocol_(protocol) {}

  Rule(const std::string& src, const std::string& chain) :
      action_(Action(NONE)), box_(DimVector()), applicable_(false),
      chain_(chain), src_(src), protocol_(PROTOCOL_WILDCARD) {}

  // XXX this constructor should be removed
  Rule(const std::string& src) : action_(Action(NONE)), box_(DimVector()),
      applicable_(false), chain_(""), src_(src), protocol_(PROTOCOL_WILDCARD) {}

  inline const Action& action() const {return action_;}

  inline const Box& box() const {return box_;}

  inline bool applicable() const {return applicable_;}

  inline const std::string& chain() const {return chain_;}

  static size_t num_distinct_rules_in_dim(const size_t dim,
      std::vector<const Rule*>& rules);

  bool operator==(const Rule& other) const;

  inline bool operator!=(const Rule& other) const {return !(*this == other);}

  inline const std::string& src() const {return src_;}

  std::string src_with_patched_chain(const std::string& chain) const;

  static void delete_rules(RuleVector& rules);

  inline dim_t protocol() const {return protocol_;}

  /*
   * Checks whether this rule is shadowed by another rule within the given
   * frame.
   */
  bool is_shadowed(const Rule* other, const Box& frame) const;

private:
  const Action action_;
  Box box_;
  const bool applicable_;
  std::string chain_;
  std::string src_;
  size_t protocol_;
};

#endif // HITABLES_RULE_HPP
