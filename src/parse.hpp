#ifndef HITABLES_PARSE_HPP
#define HITABLES_PARSE_HPP 1

#include <vector>
#include <string>
#include <sys/stat.h>
#include <fstream>
#include "rule.hpp"

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
  void parse_ruleset(const StrVector& lines, RuleVector& rules);



}

#endif // HITABLES_PARSE_HPP
