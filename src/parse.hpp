#ifndef HITABLES_PARSE_HPP
#define HITABLES_PARSE_HPP 1

#include <vector>
#include <string>

typedef std::vector<std::string> StrVector;

namespace parse {
  
  int split(const std::string& str, const std::string& sep, StrVector& parts);
  void file_read_lines(const std::string& path, StrVector& lines);

}

#endif // HITABLES_PARSE_HPP
