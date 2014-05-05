#include <cstdlib>
#include "arg.hpp"
#include <iostream>
#include <chrono>

const std::string RED("\x1b[31m");
const std::string YELLOW("\x1b[33m");
const std::string RESET("\x1b[0m");

typedef std::chrono::high_resolution_clock Clock;

void print_error(const std::string& error) {
  std::cout << std::endl << RED << "ERROR: " << error << RESET
      << std::endl << std::endl;
}


void print_usage(const std::string& path) {
  std::cout << std::endl << YELLOW << "Usage: " << path << " [--binth <NUM>] "
    << "[--spfac <NUM>] [--search <linear|binary>] "
    << "[--dim-choice <max-dist|least-max>] <PATH_TO_FILE>" << RESET
    << std::endl << std::endl;
}


inline double duration(Clock::time_point& start, Clock::time_point& end) {
  return std::chrono::duration_cast<std::chrono::duration<double>>(
      end - start).count();
}


void out_msg(const std::string& str, const bool verbose) {
  if (!verbose)
    return;
  std::cout << str << std::endl;
}


int main(int argc, char* argv[]) {
  StrVector arg_vector;
  for (int i = 1; i < argc; ++i)
    arg_vector.push_back(argv[i]);
  Arguments args;
  try {
    args = Arguments::parse_arg_vector(arg_vector);
  } catch (const std::string& msg) {
    if (msg == "usage")
      print_usage(argv[0]);
    else
      print_error(msg);
    return EXIT_FAILURE;
  }

  StrVector input;
  if (parse::file_read_lines(args.infile(), input) == 1) {
    std::stringstream ss;
    ss << "File '" << args.infile() << "' is not accessible!";
    print_error(ss.str());
    return EXIT_FAILURE;
  }

  Clock::time_point start, end;
  double time_span;
  std::stringstream msg;

  // parse rules
  RuleVector rules;
  start = Clock::now();
  parse::parse_rules(input, rules);
  end = Clock::now();
  time_span = duration(start, end);
  const size_t num_rules = rules.size();
  msg << "Parsed " << num_rules << " rule" << (num_rules > 1 ? "s" : "")
      << " in " << time_span << " seconds";
  out_msg(msg.str(), args.verbose());

  return EXIT_SUCCESS;
}
