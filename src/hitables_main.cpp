#include <cstdlib>
#include "arg.hpp"
#include <iostream>

void print_error(const std::string& error) {
  std::cout << std::endl << "ERROR: " << error << std::endl << std::endl;
}

int main(int argc, char* argv[]) {
  StrVector arg_vector;
  for (int i = 1; i < argc; ++i)
    arg_vector.push_back(argv[i]);
  Arguments args;
  try {
    args = Arguments::parse_arg_vector(arg_vector);
  } catch (const std::string& msg) {
    print_error(msg);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
