#include "arg.hpp"

inline bool is_digit(const char c) {
  return c >= 48 && c <= 57;
}


std::string trim_leading_zeros(const std::string& str) {
  const size_t len = str.size();
  size_t i = 0;
  for (; i < len; ++i)
    if (str[i] != '0')
      break;
  return str.substr(i);
}


std::string int_err_msg(const std::string& input,
    const std::string& param, const size_t min, const size_t max) {
  std::stringstream ss;
  ss << "Invalid parameter " << param << " ('" << input << "'): "
      << "must be an integer between " << min << " and " << max << "!";
  return ss.str();
}


size_t Arguments::parse_int_param(const std::string& input,
    const std::string& param, const size_t min, const size_t max) {
  std::string str(trim_leading_zeros(input));
  const size_t len = str.size();
  if (len > 5)
    throw int_err_msg(input, param, min, max);
  for (size_t i = 0; i < len; ++i)
    if (!is_digit(str[i]))
      throw int_err_msg(input, param, min, max);
  std::stringstream ss(str);
  size_t temp = 0;
  ss >> temp;
  if (temp < min || temp > max)
    throw int_err_msg(input, param, min, max);
  return temp;
}


void Arguments::parse_binth(const std::string& input) {
  binth_ = parse_int_param(input, "--binth", Arguments::MIN_BINTH,
      Arguments::MAX_BINTH);
}

void Arguments::parse_spfac(const std::string& input) {
  spfac_ = parse_int_param(input, "--spfac", Arguments::MIN_SPFAC,
      Arguments::MAX_SPFAC);
}
