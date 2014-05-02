#include "arg.hpp"

const size_t Arguments::DIM_CHOICE_MAX_DISTINCT = 0;
const size_t Arguments::DIM_CHOICE_LEAST_MAX_RULES = 1;

const size_t Arguments::SEARCH_LINEAR = 2;
const size_t Arguments::SEARCH_BINARY = 3;

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


void Arguments::parse_dim_choice(const std::string& input) {
  if (input == "max_dist")
    dim_choice_ = Arguments::DIM_CHOICE_MAX_DISTINCT;
  else if (input == "least_max")
    dim_choice_ = Arguments::DIM_CHOICE_LEAST_MAX_RULES;
  else {
    std::stringstream ss;
    ss << "Invalid parameter --dim-choice ('" << input
        << "'): must be 'max_dist' or 'least_max'!";
    throw ss.str();
  }
}


void Arguments::parse_search(const std::string& input) {
  if (input == "linear")
    search_ = Arguments::SEARCH_LINEAR;
  else if (input == "binary")
    search_ = Arguments::SEARCH_BINARY;
  else {
    std::stringstream ss;
    ss << "Invalid parameter --search ('" << input
        << "'): must be 'linear' or 'binary'!";
    throw ss.str();
  }
}
