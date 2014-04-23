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


int parse::parse_ruleset(const StrVector& lines, RuleVector& rules) {
  const size_t num_lines = lines.size();
  for (size_t i = 0; i < num_lines; ++i) {
    const std::string& line(lines[i]);
    const char first = line[0];
    if (first == ':' || first == '*' || first == '#')
      continue;
  }
  return 0;
}


inline bool is_num(const char c) {
  return c >= 48 && c <= 57;
}


uint32_t parse::parse_ip(const std::string& str) {
  StrVector parts;
  parse::split(str, ".", parts);
  const size_t num_parts = parts.size();
  if (num_parts > 4)
    throw 1;
  uint32_t ip_val = 0;
  uint32_t shift = 24;
  for (size_t i = 0; i < num_parts; ++i) {
    const std::string& octet = parts[i];
    const size_t octet_len = octet.size();
    if (octet_len == 0 || octet_len > 3)
      throw 1;
    for (size_t j = 0; j < octet_len; ++j)
      if (!is_num(octet[j]))
        throw 1;
    int octet_val;
    sscanf(octet.c_str(), "%d", &octet_val);
    if (octet_val < 0 || octet_val > 255)
      throw 1;
    ip_val |= (octet_val << shift);
    shift -= 8;
  }
  return ip_val;
}


uint32_t parse::parse_port(const std::string& port) {
  const size_t len = port.size();
  if (len > 5 || len == 0)
    throw 1;
  for (size_t i = 0; i < len; ++i)
    if (!is_num(port[i]))
      throw 1;
  int port_num;
  sscanf(port.c_str(), "%d", &port_num);
  if (port_num > 65535)
    throw 1;
  return static_cast<uint32_t>(port_num);
}


DimTuple parse::parse_port_range(const std::string& str) {
  StrVector parts;
  parse::split(str, ":", parts);
  if (parts.size() != 2)
    throw 1;
  const uint32_t port1 = parse::parse_port(parts[0]);
  const uint32_t port2 = parse::parse_port(parts[1]);
  return std::make_tuple(port1, port2);
}


DimTuple parse::parse_subnet(const std::string& str) {
  StrVector parts;
  parse::split(str, "/", parts);
  const size_t num_parts = parts.size();
  if (num_parts == 1) {
    const dim_t ip_val = parse::parse_ip(parts[0]);
    return std::make_tuple(ip_val, ip_val);
  } else if (num_parts > 2)
    throw 1;
  // we have a real subnet
  const dim_t ip_val = parse::parse_ip(parts[0]);
  // check mask
  const std::string& mask_str = parts[1];
  const size_t mask_len = mask_str.size();
  if (mask_len == 0 || mask_len > 2)
    throw 1;
  for (size_t i = 0; i < mask_len; ++i)
    if (!is_num(mask_str[i]))
      throw 1;
  int mask_val;
  sscanf(mask_str.c_str(), "%d", &mask_val);
  if (mask_val > 32)
    throw 1;
  // compute min and max ip of subnet
  if (mask_val == 0)
    return std::make_tuple(min_ip, max_ip);
  const dim_t host_len = 32 - mask_val;
  const dim_t min_ip = (ip_val >> host_len) << host_len;
  const dim_t max_ip = min_ip + ((1 << host_len) - 1);
  return std::make_tuple(min_ip, max_ip);
}


dim_t parse::parse_protocol(const std::string& str) {
  if (str == "tcp")
    return TCP;
  if (str == "udp")
    return UDP;
  if (str == "icmp")
    return ICMP;
  throw 1;
}


ActionCode parse::parse_action_code(const std::string& str) {
  if (str == "ACCEPT")
    return ACCEPT;
  if (str == "DROP")
    return DROP;
  if (str == "REJECT")
    return REJECT;
  if (str == "JUMP")
    return JUMP;
  throw 1;
}
