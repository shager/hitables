#include <cstdlib>
#include "arg.hpp"
#include <iostream>
#include <chrono>
#include "treenode.hpp"

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
    << "[--dim-choice <max-dist|least-max>] [--min-rules <NUM>] <PATH_TO_FILE>"
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
  ChainVector chains;
  start = Clock::now();
  parse::parse_rules(input, rules);
  parse::group_rules_by_chain(rules, chains);
  end = Clock::now();
  time_span = duration(start, end);
  const size_t num_rules = rules.size();
  const size_t num_chains = chains.size();
  rules.clear();
  msg << "\nParsed " << num_rules << " rule" << (num_rules != 1 ? "s" : "")
      << " (" << num_chains << " chain" << (num_chains != 1 ? "s" : "")
      << ") in " << time_span << " seconds";
  out_msg(msg.str(), args.verbose());

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
  std::stringstream domain_msg;
  domain_msg << "Extracted " << num_domains << " sub-ruleset"
      << (num_domains != 1 ? "s" : "") << " in " << time_span << " seconds";
  out_msg(domain_msg.str(), args.verbose());

  // perform HiCuts transformation
  std::vector<std::vector<TreeNode*>> chain_trees;
  const size_t dim_choice = args.dim_choice();
  start = Clock::now();
  for (size_t i_chain = 0; i_chain < num_chains; ++i_chain) {
    chain_trees.push_back(std::vector<TreeNode*>());
    for (size_t i = 0; i < num_domains; ++i) {
      const DomainTuple& domain = chain_domains[i_chain][i];
      TreeNode* tree_root = new TreeNode(chains[i_chain], domain);
      tree_root->build_tree(args.spfac(), args.binth(), dim_choice);
      chain_trees[i_chain].push_back(tree_root);
    }
  }
  end = Clock::now();
  time_span = duration(start, end);
  std::stringstream hicuts_msg;
  hicuts_msg << "Performed HiCuts transformation in " << time_span
      << " seconds";
  out_msg(hicuts_msg.str(), args.verbose());

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

  out_msg("", args.verbose());
  return EXIT_SUCCESS;
}
