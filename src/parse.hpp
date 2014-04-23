#ifndef HITABLES_PARSE_HPP
#define HITABLES_PARSE_HPP 1

#include <vector>
#include <string>
#include <sys/stat.h>
#include <fstream>
#include <cstdint>
#include "partial_ruleset.hpp"

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
   * Parses an iptables-save compliant ruleset and groups the rules to parts
   * amenable for HiCuts processing.
   * Returns 0 on success and 1 in case of parse errors.
   */
  int parse_ruleset(const StrVector& lines, RuleVector& rules);

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

}

#endif // HITABLES_PARSE_HPP
