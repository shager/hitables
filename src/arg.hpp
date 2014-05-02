#ifndef HITABLES_ARG_H
#define HITABLES_ARG_H 1

#include <string>
#include <sstream>

class Arguments {
public:
  Arguments() : binth_(0), spfac_(0) {}
  
  // binth parameter
  static const size_t MIN_BINTH = 1;
  static const size_t MAX_BINTH = 65536;
  void parse_binth(const std::string& str);
  inline size_t binth() const {return binth_;}

  // spfac parameter
  static const size_t MIN_SPFAC = 1;
  static const size_t MAX_SPFAC = 65536;
  void parse_spfac(const std::string& str);
  inline size_t spfac() const {return spfac_;}

private:
  size_t binth_;
  size_t spfac_;

  size_t parse_int_param(const std::string& input,
      const std::string& param, const size_t min, const size_t max);
};

#endif // HITABLES_ARG_H
