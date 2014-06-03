#include <cstdlib>
#include "arg.hpp"
#include <iostream>
#include <chrono>
#include "treenode.hpp"
#include "emit.hpp"

const std::string RED("\x1b[31m");
const std::string YELLOW("\x1b[33m");
const std::string RESET("\x1b[0m");

typedef std::chrono::high_resolution_clock Clock;

void print_error(const std::string& error) {
  std::cout << std::endl << RED << "ERROR: " << error << RESET
      << std::endl << std::endl;
}


void print_usage(const std::string& path) {
  std::cout << std::endl << YELLOW << "Usage: " << path << std::endl
    << "    [--binth <NUM>]" << std::endl
    << "    [--spfac <NUM>]" << std::endl
    << "    [--search <linear|binary>]" << std::endl
    << "    [--dim-choice <max-dist|least-max>]" << std::endl
    << "    [--min-rules <NUM>]" << std::endl
    << "     --infile <PATH_TO_FILE>"
    << RESET
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


/*
 * HiTables entry point
 */
int main(int argc, char* argv[]) {
  Clock::time_point total_start, total_end;
  total_start = Clock::now();

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

  srand(args.random_seed());
  Clock::time_point start, end;
  double time_span;
  std::ofstream out;
  out.open(args.outfile());
  if (!out.is_open()) {
    std::stringstream ss;
    ss << "Output file '" << args.outfile() << "' is not accessible!";
    print_error(ss.str());
    return EXIT_FAILURE;
  }

  // parse rules
  RuleVector rules;
  ChainVector chains;
  start = Clock::now();
  DefaultPolicies policies;
  parse::parse_rules(input, rules, policies);
  parse::group_rules_by_chain(rules, chains);
  end = Clock::now();
  time_span = duration(start, end);
  const size_t num_rules = rules.size();
  const size_t num_chains = chains.size();
  rules.clear();
  out << "# Parsing (" << num_rules << "): " << time_span << " seconds"
      << std::endl;

  // extract relevant sub-rulesets
  std::vector<DomainVector> chain_domains;
  start = Clock::now();
  size_t num_domains = 0;
  for (size_t i = 0; i < num_chains; ++i) {
    chain_domains.push_back(DomainVector());
    DomainVector& domains = chain_domains[i];
    parse::compute_relevant_sub_rulesets(chains[i], args.min_rules(), domains);
    num_domains += domains.size();
  }
  end = Clock::now();
  time_span = duration(start, end);
  out << "# Sub-ruleset extraction (" << num_domains << "): " << time_span
      << " seconds" << std::endl;

  // perform HiCuts transformation
  std::vector<NodeRefVector> chain_trees;
  const size_t dim_choice = args.dim_choice();
  start = Clock::now();
  for (size_t i_chain = 0; i_chain < num_chains; ++i_chain) {
    chain_trees.push_back(NodeRefVector());
    for (size_t i = 0; i < num_domains; ++i) {
      const DomainTuple& domain = chain_domains[i_chain][i];
      TreeNode* tree_root = new TreeNode(chains[i_chain], domain);
      tree_root->build_tree(args.spfac(), args.binth(), dim_choice);
      chain_trees[i_chain].push_back(tree_root);
    }
  }
  end = Clock::now();
  time_span = duration(start, end);
  out << "# HiCuts transformation: " << time_span << " seconds" << std::endl;

  // generate the output
  std::stringstream rule_out;
  rule_out << std::endl;
  std::vector<StrVector> chain_names;
  start = Clock::now();
  for (size_t i = 0; i < num_chains; ++i)
    chain_names.push_back(StrVector());
  for (size_t i = 0; i < num_chains; ++i) {
    Emitter emitter(chain_trees[i], chains[i], chain_domains[i],
        args.search());
    emitter.emit(rule_out, chain_names[i], policies);
  }

  // emit chain names
  std::stringstream chain_out;
  for (auto i = chain_names.begin(); i != chain_names.end(); ++i)
    for (auto j = i->begin(); j != i->end(); ++j)
      chain_out << ":" << *j << " - [0:0]" << std::endl;
  chain_out << std::endl;

  // emit rule code
  end = Clock::now();
  time_span = duration(start, end);
  out << "# iptables output generation: " << time_span << " seconds"
      << std::endl << std::endl;
  Emitter::emit_prefix(out, policies);
  out << chain_out.str() << rule_out.str();
  Emitter::emit_suffix(out);

  // close output stream
  out.close();

  // cleanup
  for (size_t i = 0; i < num_chains; ++i) {
    std::vector<TreeNode*>& tree_nodes = chain_trees[i];
    const size_t num_trees = tree_nodes.size();
    for (size_t j = 0; j < num_trees; ++j)
      delete tree_nodes[j];
    // delete rules
    RuleVector& chain = chains[i];
    const size_t num_rules_in_chain = chain.size();
    for (size_t j = 0; j < num_rules_in_chain; ++j)
      delete chain[j];
  }
  total_end = Clock::now();

  // write total runtime to generated file
  time_span = duration(total_start, total_end);
  StrVector generated_lines;
  parse::file_read_lines(args.outfile(), generated_lines);
  const size_t num_out_lines = generated_lines.size();
  out.open(args.outfile());
  size_t i = 0;
  for (; i < 4; ++i)
    out << generated_lines[i] << std::endl;
  out << "# Total runtime: " << time_span << " seconds" << std::endl;
  for (; i < num_out_lines; ++i)
    out << generated_lines[i] << std::endl;
  out.close();

  return EXIT_SUCCESS;
}
