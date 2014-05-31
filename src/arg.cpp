#include "arg.hpp"

const size_t Arguments::DIM_CHOICE_MAX_DISTINCT = 0;
const size_t Arguments::DIM_CHOICE_LEAST_MAX_RULES = 1;

const size_t Arguments::SEARCH_LINEAR = 2;
const size_t Arguments::SEARCH_BINARY = 3;

inline bool is_digit(const char c) {
  return c >= 48 && c <= 57;
}


bool is_digit_string(const std::string& input) {
  const size_t len = input.size();
  for (size_t i = 0; i < len; ++i)
    if (!is_digit(input[i]))
      return false;
  return true;
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
  if (input == "max-dist")
    dim_choice_ = Arguments::DIM_CHOICE_MAX_DISTINCT;
  else if (input == "least-max")
    dim_choice_ = Arguments::DIM_CHOICE_LEAST_MAX_RULES;
  else {
    std::stringstream ss;
    ss << "Invalid parameter --dim-choice ('" << input
        << "'): must be 'max-dist' or 'least-max'!";
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


void Arguments::parse_random_seed(const std::string& input) {
  const size_t random_seed(parse_int_param(input, "--random-seed", 0, 65535));
  random_seed_ = random_seed;
}


inline void check_arg_index(const size_t index, const size_t max) {
  if (index >= max)
    throw std::string(
        "Invalid number of arguments: parameter or input file missing?");
}


Arguments Arguments::parse_arg_vector(const StrVector& arg_vector) {
  const size_t num_args = arg_vector.size();
  Arguments args;
  bool infile_specified = false;
  bool outfile_specified = false;
  for (size_t i = 0; i < num_args; ++i) {
    const std::string& arg = arg_vector[i];
    if (arg == "--binth") {
      ++i;
      check_arg_index(i, num_args);
      args.parse_binth(arg_vector[i]);

    } else if (arg == "--spfac") {
      ++i;
      check_arg_index(i, num_args);
      args.parse_spfac(arg_vector[i]);

    } else if (arg == "--search") {
      ++i;
      check_arg_index(i, num_args);
      args.parse_search(arg_vector[i]);

    } else if (arg == "--dim-choice") {
      ++i;
      check_arg_index(i, num_args);
      args.parse_dim_choice(arg_vector[i]);

    } else if (arg == "--usage") {
      throw std::string("usage");

    } else if (arg == "--verbose") {
      args.set_verbose(true);

    } else if (arg == "--min-rules") {
      ++i;
      check_arg_index(i, num_args);
      args.parse_min_rules(arg_vector[i]);

    } else if (arg == "--infile") {
      ++i;
      check_arg_index(i, num_args);
      args.parse_infile(arg_vector[i]);
      infile_specified = true;

    } else if (arg == "--outfile") {
      ++i;
      check_arg_index(i, num_args);
      args.parse_outfile(arg_vector[i]);
      outfile_specified = true;

    } else if (arg == "--random-seed") {
      ++i;
      check_arg_index(i, num_args);
      args.parse_random_seed(arg_vector[i]);

    } else {
      std::stringstream ss;
      ss << "Unknown argument '" << arg << "'!";
      throw ss.str();
    }
  }
  if (!infile_specified)
    throw std::string("No input file specified (use --infile)!");
  if (!outfile_specified)
    throw std::string("No output file specified (use --outfile)!");
  return args;
}


void Arguments::parse_min_rules(const std::string& input) {
  size_t temp_min_rules = 0;
  if (!is_digit_string(input))
    goto ERROR;
  temp_min_rules = atoi(input.c_str());
  if (temp_min_rules == 0)
    goto ERROR;
  min_rules_ = temp_min_rules;
  return;

ERROR:
  std::stringstream ss;
  ss << "Invalid parameter --min-rules ('" << input << "'):"
      << " must be a positive integer!";
  throw ss.str();
}
