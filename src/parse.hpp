#ifndef HITABLES_PARSE_HPP
#define HITABLES_PARSE_HPP 1

#include <vector>
#include <string>

typedef std::vector<std::string> StrVector;

namespace parse {
  
  void file_read_lines(const std::string& path, StrVector& lines);

}

#endif // HITABLES_PARSE_HPP
