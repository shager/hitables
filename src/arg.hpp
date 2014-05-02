#ifndef HITABLES_ARG_H
#define HITABLES_ARG_H 1

#include <string>
#include "parse.hpp"
#include <sstream>

class Arguments {
public:
  Arguments() : binth_(0), spfac_(0), dim_choice_(0),
      search_(Arguments::SEARCH_LINEAR), infile_("") {}
  
  // binth parameter
  static const size_t MIN_BINTH = 1;
  static const size_t MAX_BINTH = 65536;
  inline size_t binth() const {return binth_;}

  inline void parse_binth(const std::string& input) {
    binth_ = parse_int_param(input, "--binth", Arguments::MIN_BINTH,
        Arguments::MAX_BINTH);
  }

  // spfac parameter
  static const size_t MIN_SPFAC = 1;
  static const size_t MAX_SPFAC = 65536;
  inline size_t spfac() const {return spfac_;}

  inline void parse_spfac(const std::string& input) {
    spfac_ = parse_int_param(input, "--spfac", Arguments::MIN_SPFAC,
        Arguments::MAX_SPFAC);
  }

  // dimension choice parameter
  static const size_t DIM_CHOICE_MAX_DISTINCT;
  static const size_t DIM_CHOICE_LEAST_MAX_RULES;
  inline size_t dim_choice() const {return dim_choice_;}
  void parse_dim_choice(const std::string& input);

  // search parameter
  static const size_t SEARCH_LINEAR;
  static const size_t SEARCH_BINARY;
  inline size_t search() const {return search_;}
  void parse_search(const std::string& input);
  
  // input file
  inline const std::string& infile() const {return infile_;}
  inline void parse_infile(const std::string& input) {infile_ = input;}

  /*
   * Parses an entire argument vector.
   * Returns an Arguments object in case of success or throws an std::string in
   * case of failure.
   */
  static Arguments parse_arg_vector(const StrVector& args);

private:
  size_t binth_;
  size_t spfac_;
  size_t dim_choice_;
  size_t search_;
  std::string infile_;

  size_t parse_int_param(const std::string& input,
      const std::string& param, const size_t min, const size_t max);
};

#endif // HITABLES_ARG_H
