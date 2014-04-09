#include "parse.hpp"

int parse::split(const std::string& str, const std::string& sep,
    StrVector& parts) {

  const size_t sep_len = sep.size();
  if (sep_len == 0)
    return 1;
  size_t start = 0;
  size_t index = 0;
  while ((index = str.find(sep, start)) != std::string::npos) {
    const size_t len = index - start;
    parts.push_back(str.substr(start, len));
    start = index + sep_len;
  }
  parts.push_back(str.substr(start));
  return 0;
}
