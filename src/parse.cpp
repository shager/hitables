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


inline bool is_ws_char(const char c) {
  return (c == ' ' || c == '\n' || c == '\t' || c == '\r');
}


void parse::trim(std::string& str) {
  int len = str.size();
  int i = 0;
  for (; i < len; ++i)
    if (!is_ws_char(str[i]))
      break;
  const int start = i;
  if (start == len) {
    str = "";
    return;
  }
  for (i = len - 1; i >= 0; --i)
    if (!is_ws_char(str[i]))
      break;
  const int end = i;
  str = str.substr(start, end - start + 1);
}


inline bool file_exists(const std::string& path) {
  struct stat buffer;
  return stat(path.c_str(), &buffer) == 0;
}


int parse::file_read_lines(const std::string& path, StrVector& lines) {
  if (!file_exists(path))
    return 1;
  std::ifstream in;
  std::string line;
  in.open(path);
  while (in.good()) {
    std::getline(in, line);
    parse::trim(line);
    if (line.size() == 0)
      continue;
    lines.push_back(line);
  }
  in.close();
  return 0;
}
