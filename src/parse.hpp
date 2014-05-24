#ifndef HITABLES_PARSE_HPP
#define HITABLES_PARSE_HPP 1

#include <vector>
#include <string>
#include <sys/stat.h>
#include <fstream>
#include <cstdint>
#include "rule.hpp"
#include <unordered_map>
#include <algorithm>

typedef std::vector<std::string> StrVector;

namespace parse {

  int split(const std::string& str, const std::string& sep, StrVector& parts);

  void trim(std::string& str);

  /*
   * Reads a file line by line and returns all non-empty lines in the given
   * vector.
   * Returns 0 on success and 1 in case of a non-accessible file.
   */
  int file_read_lines(const std::string& path, StrVector& lines);

  /*
   * Parses an IPv4 address in dotted decimal notation.
   * Returns the numeric representation of the IPv4 address.
   * Throws 1 in case of failure.
   */
  uint32_t parse_ip(const std::string& str);

  /*
   * Parses a port number.
   * Returns the numeric representation of the port number.
   * Throws 1 in case of failure.
   */
  uint32_t parse_port(const std::string& str);

  /*
   * Parses a port range of the form <START>:<END>.
   * Returns a DimTuple in case of success.
   * Throws 1 in case of failure.
   */
  DimTuple parse_port_range(const std::string& str);

  /*
   * Parses a subnet string of the form <IP_ADDR>/<NUM_MASK_BITS>.
   * Returns a tuple of the minimum and maximum IP addresses in case of success.
   * Throws 1 in case of failure.
   */
  DimTuple parse_subnet(const std::string& str);
  
  /*
   * Parses an IP range of the form <IP>-<IP>.
   * Returns a DimTuple in case of success.
   * Throws 1 in case of failure.
   */
  DimTuple parse_ip_range(const std::string& str);

  /*
   * Parses a protocol string.
   * Returns an identifier for the protocol in case of sucess.
   * Throws 1 in case of failure.
   */
  dim_t parse_protocol(const std::string& str);


  /*
   * Parses an action string such as ACCEPT, DROP, REJECT, JUMP.
   * Throws 1 in case of failure.
   */
  ActionCode parse_action_code(const std::string& str);

  /*
   * Checks whether the iptables-save rule is applicable to HiTables usage.
   * Returns a rule object that states whether it is valid or not.
   * Expects the rule to be whitespace-split.
   */
  Rule* parse_rule(const std::string& input);

  /*
   * Parses the given input string into a vector of rule objects.
   */
  void parse_rules(const StrVector& input, RuleVector& rules);

  /*
   * Groups the given rules according to their chains.
   * The result is a vector of rulevectors.
   */
  void group_rules_by_chain(const RuleVector& rules, ChainVector& chains);

  /*
   * Computes a vector of size_t tuples that indicate the start and end indices
   * of HiTables-relevant sub rulesets.
   */
  void compute_relevant_sub_rulesets(RuleVector& rules, const size_t min_rules,
      DomainVector& domains);
}

#endif // HITABLES_PARSE_HPP
