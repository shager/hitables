#include <cstdlib>
#include "arg.hpp"
#include <iostream>

const std::string RED("\x1b[31m");
const std::string YELLOW("\x1b[33m");
const std::string RESET("\x1b[0m");


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

  return EXIT_SUCCESS;
}
