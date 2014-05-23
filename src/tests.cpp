#include "box.hpp"
#include "treenode.hpp"
#include "parse.hpp"
#include "arg.hpp"
#include <cstdio>
#include "emit.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE hitables_tests
#include <boost/test/unit_test.hpp>

using namespace std;

#define SINGLE_DIM_NODE_WITH_THREE_RULES          \
  DimVector bounds;                               \
  bounds.push_back(make_tuple(0, 10));            \
  TreeNode node(bounds);                          \
                                                  \
  DimVector rule1_bounds;                         \
  rule1_bounds.push_back(make_tuple(1, 3));       \
  Box rule1_box(rule1_bounds);                    \
  Rule rule1(DROP, rule1_box, "");                    \
                                                  \
  DimVector rule2_bounds;                         \
  rule2_bounds.push_back(make_tuple(4, 7));       \
  Box rule2_box(rule2_bounds);                        \
  Rule rule2(DROP, rule2_box, "");                            \
                               \
  DimVector rule3_bounds;                         \
  rule3_bounds.push_back(make_tuple(8, 10));      \
  Box rule3_box(rule3_bounds);                        \
  Rule rule3(DROP, rule3_box, "");                    \
                                                  \
  node.add_rule(static_cast<const Rule*>(&rule1)); \
  node.add_rule(static_cast<const Rule*>(&rule2)); \
  node.add_rule(static_cast<const Rule*>(&rule3));


/*****************************************************************************
 *                            B O X   T E S T S                              *
 *****************************************************************************/

BOOST_AUTO_TEST_CASE(box_cut_simple) {
  DimVector bounds;
  bounds.push_back(make_tuple(0, 4));

  Box box(bounds);
  std::vector<Box> boxes;

  for (size_t i = 1; i <= 3; ++i) {
    box.cut(0, i, boxes);
    BOOST_CHECK_EQUAL(boxes.size(), i + 1);
    boxes.clear();
  }

  box.cut(0, 2, boxes);
  BOOST_CHECK_EQUAL(0, get<0>(boxes[0].box_bounds()[0]));
  BOOST_CHECK_EQUAL(1, get<1>(boxes[0].box_bounds()[0]));
  BOOST_CHECK_EQUAL(2, get<0>(boxes[1].box_bounds()[0]));
  BOOST_CHECK_EQUAL(3, get<1>(boxes[1].box_bounds()[0]));
  BOOST_CHECK_EQUAL(4, get<0>(boxes[2].box_bounds()[0]));
  BOOST_CHECK_EQUAL(4, get<1>(boxes[2].box_bounds()[0]));
}


BOOST_AUTO_TEST_CASE(box_collide_one_dimension) {
  DimVector bounds1;
  bounds1.push_back(make_tuple(0, 2));
  Box box1(bounds1);
  
  DimVector bounds2;
  bounds2.push_back(make_tuple(2, 3));
  Box box2(bounds2);

  BOOST_CHECK(box1.collide(box2));
  BOOST_CHECK(box2.collide(box1));

  DimVector bounds3;
  bounds3.push_back(make_tuple(3, 5));
  Box box3(bounds3);

  BOOST_CHECK(!box1.collide(box3));
  BOOST_CHECK(!box3.collide(box1));

  BOOST_CHECK(box3.collide(box2));
  BOOST_CHECK(box2.collide(box3));
}


BOOST_AUTO_TEST_CASE(box_collide_two_dimensions) {
  DimVector bounds1;
  bounds1.push_back(make_tuple(0, 3));
  bounds1.push_back(make_tuple(0, 2));
  Box box1(bounds1);

  DimVector bounds2;
  bounds2.push_back(make_tuple(2, 4));
  bounds2.push_back(make_tuple(1, 3));
  Box box2(bounds2);

  BOOST_CHECK(box1.collide(box2));
  BOOST_CHECK(box2.collide(box1));

  DimVector bounds3;
  bounds3.push_back(make_tuple(3, 5));
  bounds3.push_back(make_tuple(2, 4));
  Box box3(bounds3);
  
  BOOST_CHECK(box1.collide(box3));
  BOOST_CHECK(box3.collide(box1));

  DimVector bounds4;
  bounds4.push_back(make_tuple(4, 6));
  bounds4.push_back(make_tuple(2, 4));
  Box box4(bounds4);
  
  BOOST_CHECK(!box1.collide(box4));
  BOOST_CHECK(!box4.collide(box1));
}


BOOST_AUTO_TEST_CASE(box_num_distinct_boxes_in_dim) {
  DimVector rule1_bounds;
  rule1_bounds.push_back(make_tuple(1, 2));
  Box rule1(rule1_bounds);

  DimVector rule2_bounds;
  rule2_bounds.push_back(make_tuple(3, 4));
  Box rule2(rule2_bounds);

  DimVector rule3_bounds;
  rule3_bounds.push_back(make_tuple(5, 6));
  Box rule3(rule3_bounds);

  std::vector<const Box*> rules;
  rules.push_back(&rule1);
  rules.push_back(&rule2);
  rules.push_back(&rule3);
  BOOST_CHECK_EQUAL(Box::num_distinct_boxes_in_dim(0, rules), 3);

  DimVector rule4_bounds;
  rule4_bounds.push_back(make_tuple(1, 6));
  Box rule4(rule4_bounds);

  rules.push_back(&rule4);
  BOOST_CHECK_EQUAL(Box::num_distinct_boxes_in_dim(0, rules), 0);
}

/*****************************************************************************
 *                         A C T I O N   T E S T S                           *
 *****************************************************************************/

BOOST_AUTO_TEST_CASE(action_assignment) {
  Action a1(REJECT);
  Action a2(a1);
  BOOST_CHECK(a1.code() == a2.code());
  BOOST_CHECK(a1.next_chain() == a2.next_chain());

  Action a3(DROP, "abc");
  a2 = a3;
  BOOST_CHECK(a2.code() == DROP);
  BOOST_CHECK(a2.next_chain() == "abc");
}

/*****************************************************************************
 *                       T R E E N O D E   T E S T S                         *
 *****************************************************************************/

BOOST_AUTO_TEST_CASE(treenode_cut) {
  SINGLE_DIM_NODE_WITH_THREE_RULES;
  node.cut(0, 1);
  const std::vector<TreeNode>& children = node.children();
  BOOST_CHECK_EQUAL(children.size(), 2);
  BOOST_CHECK(*children[0].rules()[0] == rule1);
  BOOST_CHECK(*children[0].rules()[1] == rule2);
  BOOST_CHECK(*children[1].rules()[0] == rule2);
  BOOST_CHECK(*children[1].rules()[1] == rule3);
}


BOOST_AUTO_TEST_CASE(treenode_cut_small_rules) {
  DimVector bounds;
  bounds.push_back(make_tuple(0, 10));
  TreeNode node(bounds);
  std::vector<Rule*> rule_pointers;
  for (size_t i = 0; i < 10; ++i) {
    DimVector rule_bounds;
    rule_bounds.push_back(make_tuple(i, i));
    Rule* rule = new Rule(DROP, Box(rule_bounds), "");
    node.add_rule(rule);
    rule_pointers.push_back(rule);
  }
  node.cut(0, 4);
  const std::vector<TreeNode>& children = node.children();
  BOOST_CHECK_EQUAL(children.size(), 5);
  BOOST_CHECK_EQUAL(children[0].rules().size(), 3);
  for (size_t i = 0; i < 10; ++i)
    delete rule_pointers[i];
}


BOOST_AUTO_TEST_CASE(treenode_space_measure) {
  SINGLE_DIM_NODE_WITH_THREE_RULES;
  // the node has not been cut, so space measure should equal 0
  BOOST_CHECK_EQUAL(node.space_measure(), 0);
  node.cut(0, 1);
  // node has two children with two rules each, so space measure should equal 6
  BOOST_CHECK_EQUAL(node.space_measure(), 6);
}


BOOST_AUTO_TEST_CASE(treenode_reset_cut) {
  SINGLE_DIM_NODE_WITH_THREE_RULES;
  node.cut(0, 1);
  // node has been cut, so it should have child nodes
  BOOST_CHECK(!node.children().empty());
  node.reset_cut();
  // the cut has been reset, so there should be no child nodes
  BOOST_CHECK(node.children().empty());
  // cut the node again, so it should have children
  node.cut(0, 1);
  BOOST_CHECK(!node.children().empty());
}


BOOST_AUTO_TEST_CASE(treenode_space_measure_upper_bound) {
  SINGLE_DIM_NODE_WITH_THREE_RULES;
  for (size_t spfac = 1; spfac <= 10; ++spfac)
    BOOST_CHECK_EQUAL(node.space_measure_upper_bound(spfac), spfac * 3);
}


BOOST_AUTO_TEST_CASE(treenode_determine_number_of_cuts) {
  SINGLE_DIM_NODE_WITH_THREE_RULES;
  BOOST_CHECK_EQUAL(node.determine_number_of_cuts(0, 1), 4);
  BOOST_CHECK_EQUAL(node.determine_number_of_cuts(0, 2), 4);

  DimVector bounds2;
  bounds2.push_back(make_tuple(0, 10));
  TreeNode node2(bounds2);
  std::vector<Rule*> rule_pointers;
  for (size_t i = 0; i < 10; ++i) {
    DimVector rule_bounds;
    rule_bounds.push_back(make_tuple(i, i));
    Rule* rule = new Rule(DROP, Box(rule_bounds), "");
    node2.add_rule(rule);
    rule_pointers.push_back(rule);
  }
  BOOST_CHECK_EQUAL(node2.determine_number_of_cuts(0, 1), 4);
  BOOST_CHECK_EQUAL(node2.determine_number_of_cuts(0, 2), 9);
  for (size_t i = 0; i < 10; ++i)
    delete rule_pointers[i];
}


BOOST_AUTO_TEST_CASE(treenode_dim_max_distinct_rules) {
  DimVector bounds;
  bounds.push_back(make_tuple(0, 10));
  bounds.push_back(make_tuple(0, 10));
  TreeNode node(bounds);

  DimVector rule1_bounds;
  rule1_bounds.push_back(make_tuple(1, 5));
  rule1_bounds.push_back(make_tuple(0, 1));
  Rule rule1(DROP, Box(rule1_bounds), "");
  node.add_rule(&rule1);

  DimVector rule2_bounds;
  rule2_bounds.push_back(make_tuple(6, 9));
  rule2_bounds.push_back(make_tuple(0, 1));
  Rule rule2(DROP, Box(rule2_bounds), "");
  node.add_rule(&rule2);

  const size_t dim = node.dim_max_distinct_rules();
  BOOST_CHECK_EQUAL(dim, 0);

  TreeNode node2(bounds);

  DimVector rule3_bounds;
  rule3_bounds.push_back(make_tuple(0, 1));
  rule3_bounds.push_back(make_tuple(1, 5));
  Rule rule3(DROP, Box(rule3_bounds), "");
  node2.add_rule(&rule3);

  DimVector rule4_bounds;
  rule4_bounds.push_back(make_tuple(0, 1));
  rule4_bounds.push_back(make_tuple(6, 9));
  Rule rule4(DROP, Box(rule4_bounds), "");
  node2.add_rule(&rule4);

  const size_t dim2 = node2.dim_max_distinct_rules();
  BOOST_CHECK_EQUAL(dim2, 1);
}


BOOST_AUTO_TEST_CASE(treenode_dim_least_max_rules_per_child) {
  DimVector bounds;
  bounds.push_back(make_tuple(0, 10));
  bounds.push_back(make_tuple(0, 10));
  TreeNode node(bounds);
  
  DimVector rule1_bounds;
  rule1_bounds.push_back(make_tuple(0, 10));
  rule1_bounds.push_back(make_tuple(1, 1));
  Rule rule1(DROP, Box(rule1_bounds), "");
  node.add_rule(&rule1);

  DimVector rule2_bounds;
  rule2_bounds.push_back(make_tuple(0, 10));
  rule2_bounds.push_back(make_tuple(8, 8));
  Rule rule2(DROP, Box(rule2_bounds), "");
  node.add_rule(&rule2);

  const size_t dim = node.dim_least_max_rules_per_child(1);
  BOOST_CHECK_EQUAL(dim, 1);
}


BOOST_AUTO_TEST_CASE(treenode_random_dim) {
  DimVector bounds;
  bounds.push_back(make_tuple(0, 10));
  bounds.push_back(make_tuple(0, 10));
  TreeNode node(bounds);

  for (size_t i = 0; i < 20; ++i) {
    const size_t dim = node.random_dim();
    BOOST_CHECK(dim == 0 || dim == 1);
  }
}


BOOST_AUTO_TEST_CASE(treenode_rule_ctor) {
  RuleVector rules;
  DimVector v1;
  v1.push_back(make_tuple(1, 3));
  v1.push_back(make_tuple(2, 5));
  Box box1(v1);
  rules.push_back(new Rule(DROP, box1, ""));

  DimVector v2;
  v2.push_back(make_tuple(0, 11));
  v2.push_back(make_tuple(5, 15));
  Box box2(v2);
  rules.push_back(new Rule(DROP, box2, ""));

  DimVector v3;
  v3.push_back(make_tuple(0, 20));
  v3.push_back(make_tuple(4, 30));
  Box box3(v3);
  rules.push_back(new Rule(DROP, box3, ""));

  TreeNode node1(rules, make_tuple(0, 1));
  BOOST_CHECK_EQUAL(node1.num_rules(), 2);
  DimVector db1;
  db1.push_back(make_tuple(0, 11));
  db1.push_back(make_tuple(2, 15));
  Box bounding_box1(db1);
  BOOST_CHECK(node1.box() == bounding_box1);

  TreeNode node2(rules, make_tuple(1, 2));
  BOOST_CHECK_EQUAL(node2.num_rules(), 2);
  DimVector db2;
  db2.push_back(make_tuple(0, 20));
  db2.push_back(make_tuple(4, 30));
  Box bounding_box2(db2);
  BOOST_CHECK(node2.box() == bounding_box2);

  TreeNode node3(rules, make_tuple(0, 2));
  BOOST_CHECK_EQUAL(node3.num_rules(), 3);
  DimVector db3;
  db3.push_back(make_tuple(0, 20));
  db3.push_back(make_tuple(2, 30));
  Box bounding_box3(db3);
  BOOST_CHECK(node3.box() == bounding_box3);

  for (size_t i = 0; i < rules.size(); ++i)
    delete rules[i];
}


BOOST_AUTO_TEST_CASE(treenode_prot) {
  const size_t num = 2;
  std::string prots[num] = {"tcp", "udp"};
  for (size_t j = 0; j < num; ++j) {
    stringstream ss;
    ss << "-A x -p " << prots[j] << " -j DROP";
    RuleVector rules;
    rules.push_back(parse::parse_rule(ss.str()));
    TreeNode node(rules, make_tuple(0, 0));
    BOOST_CHECK_EQUAL(node.prot(), prots[j]);
    Rule::delete_rules(rules);
  }
}

/*****************************************************************************
 *                         P A R S E   T E S T S                             *
 *****************************************************************************/

BOOST_AUTO_TEST_CASE(parse_split) {
  StrVector res;
  BOOST_CHECK_EQUAL(parse::split("a b c", " ", res), 0);
  BOOST_CHECK_EQUAL(res.size(), 3);
  BOOST_CHECK(res.at(0) == "a");
  BOOST_CHECK(res.at(1) == "b");
  BOOST_CHECK(res.at(2) == "c");
  res.clear();

  BOOST_CHECK_EQUAL(parse::split("a b c", "x", res), 0);
  BOOST_CHECK_EQUAL(res.size(), 1);
  BOOST_CHECK(res.at(0) == "a b c");
  res.clear();

  BOOST_CHECK_EQUAL(parse::split("aa", "a", res), 0);
  BOOST_CHECK_EQUAL(res.size(), 3);
  BOOST_CHECK(res.at(0) == "");
  BOOST_CHECK(res.at(1) == "");
  BOOST_CHECK(res.at(2) == "");
  res.clear();

  BOOST_CHECK_EQUAL(parse::split("", "a", res), 0);
  BOOST_CHECK_EQUAL(res.size(), 1);
  BOOST_CHECK(res.at(0) == "");
  res.clear();

  BOOST_CHECK_EQUAL(parse::split("abc", "", res), 1);
}

BOOST_AUTO_TEST_CASE(parse_trim) {
  string s("  a");
  parse::trim(s);
  BOOST_CHECK(s == "a");

  s = "a";
  parse::trim(s);
  BOOST_CHECK(s == "a");

  s = "a  ";
  parse::trim(s);
  BOOST_CHECK(s == "a");

  s = " \r \n a \n\r\t a  \n \t";
  parse::trim(s);
  BOOST_CHECK(s == "a \n\r\t a");

  s = "";
  parse::trim(s);
  BOOST_CHECK(s == "");

  s = "   ";
  parse::trim(s);
  BOOST_CHECK(s == "");

  s = "    \n \r \t   ";
  parse::trim(s);
  BOOST_CHECK(s == "");
}

BOOST_AUTO_TEST_CASE(parse_file_read_lines) {
  ofstream out;
  const string fn("___TEST_FILE___");
  out.open(fn);
  out << "a\nb\n\n   \n\nc\n";
  out.close();

  StrVector lines;
  BOOST_CHECK_EQUAL(parse::file_read_lines(fn, lines), 0);
  BOOST_CHECK_EQUAL(lines.size(), 3);
  BOOST_CHECK(lines[0] == "a");
  BOOST_CHECK(lines[1] == "b");
  BOOST_CHECK(lines[2] == "c");
  lines.clear();
  std::remove(fn.c_str());

  out.open(fn);
  out.close();
  BOOST_CHECK_EQUAL(parse::file_read_lines(fn, lines), 0);
  BOOST_CHECK_EQUAL(lines.size(), 0);
  lines.clear();
  std::remove(fn.c_str());

  const string na("___SOME_VERY_NONEXISTING_FILE___");
  BOOST_CHECK_EQUAL(parse::file_read_lines(na, lines), 1);
  BOOST_CHECK_EQUAL(lines.size(), 0);
}


BOOST_AUTO_TEST_CASE(parse_parse_ip) {
  BOOST_CHECK_EQUAL(parse::parse_ip("1.2.3.4"), 16909060);
  BOOST_CHECK_EQUAL(parse::parse_ip("0.00.000.0"), 0);
  BOOST_CHECK_EQUAL(parse::parse_ip("255.255.255.255"), 4294967295);
  StrVector faults;
  faults.push_back("asdasd");
  faults.push_back("1.2.3.4.5");
  faults.push_back("1..2.3");
  faults.push_back(" 1.2.3.4");
  faults.push_back("1.2.3.4 ");
  faults.push_back("1. 2.3");
  faults.push_back("255.255.256.255");
  for (size_t i = 0; i < faults.size(); ++i) {
    bool thrown = false;
    try {
      parse::parse_ip(faults[i]);
    } catch (const string& msg) {
      BOOST_CHECK(msg == "Invalid subnet: '" + faults[i] + "'");
      thrown = true;
    }
    BOOST_CHECK(thrown);
  }
}


BOOST_AUTO_TEST_CASE(parse_parse_ip_prefix) {
  BOOST_CHECK_EQUAL(parse::parse_ip("1.0.0.0"), parse::parse_ip("1"));
  BOOST_CHECK_EQUAL(parse::parse_ip("1.0.0.0"), parse::parse_ip("1.0"));
  BOOST_CHECK_EQUAL(parse::parse_ip("1.0.0.0"), parse::parse_ip("1.0.0"));
}


BOOST_AUTO_TEST_CASE(parse_parse_port) {
  BOOST_CHECK_EQUAL(parse::parse_port("1"), 1);
  BOOST_CHECK_EQUAL(parse::parse_port("65535"), 65535);
  BOOST_CHECK_EQUAL(parse::parse_port("0"), 0);
  BOOST_CHECK_EQUAL(parse::parse_port("123"), 123);
  std::vector<std::string> faults;
  faults.push_back("asdasdas");
  faults.push_back("-1");
  faults.push_back("  1  ");
  faults.push_back("0001111");
  faults.push_back("65536");
  faults.push_back("");
  for (size_t i = 0; i < faults.size(); ++i) {
    bool thrown = false;
    try {
      parse::parse_port(faults[i]);
    } catch (const std::string& msg) {
      BOOST_CHECK(msg == ("Invalid port: '" + faults[i] + "'"));
      thrown = true;
    }
    BOOST_CHECK(thrown);
  }
}


BOOST_AUTO_TEST_CASE(parse_parse_port_range) {
  DimTuple t1 = parse::parse_port_range("123:456");
  BOOST_CHECK_EQUAL(get<0>(t1), 123);
  BOOST_CHECK_EQUAL(get<1>(t1), 456);

  DimTuple t2 = parse::parse_port_range("0:65535");
  BOOST_CHECK_EQUAL(get<0>(t2), 0);
  BOOST_CHECK_EQUAL(get<1>(t2), 65535);

  std::vector<std::string> faults;
  faults.push_back("");
  faults.push_back(":1");
  faults.push_back("1:");
  faults.push_back("1 :1");
  faults.push_back("1: 1");
  faults.push_back(" 1:1");
  faults.push_back("1:1 ");
  for (size_t i = 0; i < faults.size(); ++i) {
    bool thrown = false;
    try {
      parse::parse_port_range(faults[i]);
    } catch (const std::string& msg) {
      BOOST_CHECK(msg.find("Invalid port") == 0);
      thrown = true;
    }
    BOOST_CHECK(thrown);
  }
}


BOOST_AUTO_TEST_CASE(parse_parse_subnet) {
  DimTuple t1 = parse::parse_subnet("1.2.3.4/24");
  BOOST_CHECK_EQUAL(get<0>(t1), parse::parse_ip("1.2.3.0"));
  BOOST_CHECK_EQUAL(get<1>(t1), parse::parse_ip("1.2.3.255"));

  DimTuple t2 = parse::parse_subnet("1.2.3.4");
  BOOST_CHECK_EQUAL(get<0>(t2), parse::parse_ip("1.2.3.4"));
  BOOST_CHECK_EQUAL(get<1>(t2), parse::parse_ip("1.2.3.4"));

  DimTuple t3 = parse::parse_subnet("1/8");
  BOOST_CHECK_EQUAL(get<0>(t3), parse::parse_ip("1.0.0.0"));
  BOOST_CHECK_EQUAL(get<1>(t3), parse::parse_ip("1.255.255.255"));

  DimTuple t4 = parse::parse_subnet("1/24");
  BOOST_CHECK_EQUAL(get<0>(t4), parse::parse_ip("1.0.0.0"));
  BOOST_CHECK_EQUAL(get<1>(t4), parse::parse_ip("1.0.0.255"));

  DimTuple t5 = parse::parse_subnet("1/0");
  BOOST_CHECK_EQUAL(get<0>(t5), parse::parse_ip("0.0.0.0"));
  BOOST_CHECK_EQUAL(get<1>(t5), parse::parse_ip("255.255.255.255"));

  DimTuple t6 = parse::parse_subnet("128/1");
  BOOST_CHECK_EQUAL(get<0>(t6), 1 << 31);
  BOOST_CHECK_EQUAL(get<1>(t6), parse::parse_ip("255.255.255.255"));

  StrVector faults;
  faults.push_back("asdsad");
  faults.push_back("1//4");
  faults.push_back(" 1.2.3.4/5");
  faults.push_back("1.2.3.4/5 ");
  faults.push_back("1/2/3");
  faults.push_back("1.2.3.4/33");
  faults.push_back("1.2.3.4/001");
  for (size_t i = 0; i < faults.size(); ++i) {
    bool thrown = false;
    try {
      parse::parse_subnet(faults[i]);
    } catch (const std::string& msg) {
      thrown = true;
    }
    BOOST_CHECK(thrown);
  }
}


BOOST_AUTO_TEST_CASE(parse_parse_protocol) {
  BOOST_CHECK_EQUAL(parse::parse_protocol("tcp"), TCP);
  BOOST_CHECK_EQUAL(parse::parse_protocol("udp"), UDP);

  StrVector failures;
  failures.push_back("xxx");
  failures.push_back("icmp");
  for (size_t i = 0; i < failures.size(); ++i) {
    bool thrown = false;
    try {
      parse::parse_protocol("xxx");
    } catch (const std::string& msg) {
      BOOST_CHECK(msg == "Invalid protocol: 'xxx'");
      thrown = true;
    }
    BOOST_CHECK(thrown);
  }
}


BOOST_AUTO_TEST_CASE(parse_parse_action_code) {
  BOOST_CHECK_EQUAL(parse::parse_action_code("DROP"), DROP);
  BOOST_CHECK_EQUAL(parse::parse_action_code("ACCEPT"), ACCEPT);
  BOOST_CHECK_EQUAL(parse::parse_action_code("REJECT"), REJECT);
  BOOST_CHECK_EQUAL(parse::parse_action_code("JUMP"), JUMP);

  bool thrown = false;
  try {
    parse::parse_action_code("xxx");
  } catch (const std::string& msg) {
    BOOST_CHECK(msg ==  "Invalid action code: 'xxx'");
    thrown = true;
  }
  BOOST_CHECK(thrown);
}


BOOST_AUTO_TEST_CASE(parse_parse_ip_range) {
  DimTuple t1(parse::parse_ip_range("0.0.0.1-0.0.0.2"));
  BOOST_CHECK_EQUAL(get<0>(t1), 1);
  BOOST_CHECK_EQUAL(get<1>(t1), 2);

  StrVector faults;
  faults.push_back("asdasdasd");
  faults.push_back("0.0.0.1-");
  faults.push_back("-0.0.0.1");
  bool thrown;
  for (size_t i = 0; i < faults.size(); ++i) {
    try {
      parse::parse_ip_range(faults[i]);
    } catch (const string& msg) {
      thrown = true;
    }
    BOOST_CHECK(thrown);
  }
}


BOOST_AUTO_TEST_CASE(parse_parse_rule) {
  Rule* rule;
  rule = parse::parse_rule("-A INPUT -p udp -j ACCEPT");
  BOOST_CHECK(rule->applicable());
  delete rule;

  rule = parse::parse_rule("-A INPUT -p tcp -j ACCEPT");
  BOOST_CHECK(rule->applicable());
  delete rule;

  rule = parse::parse_rule("-A adasdas -p udp -j ACCEPT");
  BOOST_CHECK(rule->applicable());
  delete rule;

  rule = parse::parse_rule("-p udp -j ACCEPT");
  BOOST_CHECK(!rule->applicable());
  delete rule;

  bool thrown = false;
  try {
    parse::parse_rule("-A INPUT -p asdasd -j ACCEPT");
  } catch (const string& msg) {
    thrown = true;
  }
  BOOST_CHECK(thrown);

  rule = parse::parse_rule("-A INPUT -blabla -j ACCEPT");
  BOOST_CHECK(!rule->applicable());
  delete rule;

  thrown = false;
  try {
    parse::parse_rule("-A INPUT -p tcp -j asdjasdkj");
  } catch (const string& msg) {
    thrown = true;
  }
  BOOST_CHECK(thrown);

  rule = parse::parse_rule("-A INPUT -p udp -m iprange --src-range 117.159.160.68-117.159.164.152 --dst-range 253.59.172.172-253.59.175.252 -m udp --sport 38435:39668 --dport 14309:14373 -j ACCEPT");
  BOOST_CHECK(rule->applicable());
  delete rule;

  rule = parse::parse_rule("-A INPUT --src 1.2.3.4 -j DROP");
  BOOST_CHECK(!rule->applicable());
  delete rule;
}


BOOST_AUTO_TEST_CASE(parse_parse_rules_values) {
  Rule* rule = parse::parse_rule("-A INPUT -p udp -m iprange --src-range 0.0.0.5-0.0.0.6 --dst-range 0.0.0.7-0.0.0.8 -m udp --sport 1:2 --dport 3:4 -j ACCEPT");
  BOOST_CHECK(rule->applicable());
  BOOST_CHECK(rule->action().code() == ACCEPT);
  // check rule size
  const DimVector& bounds = rule->box().box_bounds();
  BOOST_CHECK_EQUAL(bounds.size(), 5);
  BOOST_CHECK(get<0>(bounds[0]) == 1);
  BOOST_CHECK(get<1>(bounds[0]) == 2);
  BOOST_CHECK(get<0>(bounds[1]) == 3);
  BOOST_CHECK(get<1>(bounds[1]) == 4);
  BOOST_CHECK(get<0>(bounds[2]) == 5);
  BOOST_CHECK(get<1>(bounds[2]) == 6);
  BOOST_CHECK(get<0>(bounds[3]) == 7);
  BOOST_CHECK(get<1>(bounds[3]) == 8);
  BOOST_CHECK(get<0>(bounds[4]) == UDP);
  BOOST_CHECK(get<1>(bounds[4]) == UDP);
  BOOST_CHECK(rule->chain() == "INPUT");
  delete rule;
}


BOOST_AUTO_TEST_CASE(parse_parse_rules_prefixes) {
  Rule* rule = parse::parse_rule("-A INPUT --src 0.0.0.5/32 --dst 0.0.0.3/31 -p udp --sport 1:2 --dport 3:4 -j DROP");
  BOOST_CHECK(rule->applicable());
  BOOST_CHECK(rule->action().code() == DROP);
  // check rule size
  const DimVector& bounds = rule->box().box_bounds();
  BOOST_CHECK_EQUAL(bounds.size(), 5);
  BOOST_CHECK(get<0>(bounds[0]) == 1);
  BOOST_CHECK(get<1>(bounds[0]) == 2);
  BOOST_CHECK(get<0>(bounds[1]) == 3);
  BOOST_CHECK(get<1>(bounds[1]) == 4);
  BOOST_CHECK(get<0>(bounds[2]) == 5);
  BOOST_CHECK(get<1>(bounds[2]) == 5);
  BOOST_CHECK(get<0>(bounds[3]) == 2);
  BOOST_CHECK(get<1>(bounds[3]) == 3);
  BOOST_CHECK(get<0>(bounds[4]) == UDP);
  BOOST_CHECK(get<1>(bounds[4]) == UDP);
  BOOST_CHECK(rule->chain() == "INPUT");
  delete rule;
}


BOOST_AUTO_TEST_CASE(parse_parse_rules) {
  const size_t n = 10;
  StrVector input;
  const std::string rule("-A INPUT --src 0.0.0.5/32 --dst 0.0.0.3/31 -j DROP");
  for (size_t i = 0; i < n; ++i)
    input.push_back(rule);
  RuleVector rules;
  parse::parse_rules(input, rules);
  BOOST_CHECK_EQUAL(rules.size(), 10);
  Rule::delete_rules(rules);
}


BOOST_AUTO_TEST_CASE(parse_parse_rules_with_meta_info) {
  StrVector input;
  input.push_back("# 1");
  input.push_back("# 2");
  input.push_back("*filter");
  input.push_back(":INPUT [0:0]");
  input.push_back(":FORWARD [0:0]");
  input.push_back(":OUTPUT [0:0]");
  input.push_back("-A c -j DROP");
  input.push_back("COMMIT");
  input.push_back("# 3");
  RuleVector rules;
  parse::parse_rules(input, rules);
  BOOST_CHECK_EQUAL(rules.size(), 1);
  Rule::delete_rules(rules);
}


BOOST_AUTO_TEST_CASE(parse_compute_relevant_sub_rulesets) {
  RuleVector rules;
  DomainVector domains;
  parse::compute_relevant_sub_rulesets(rules, 2, domains);
  BOOST_CHECK(domains.empty());

  rules.push_back(new Rule("")); // 0
  rules.push_back(new Rule("")); // 1
  parse::compute_relevant_sub_rulesets(rules, 2, domains);
  BOOST_CHECK(domains.empty());
  
  DimVector dims;
  dims.push_back(make_tuple(1, 1));
  dims.push_back(make_tuple(2, 3));
  dims.push_back(make_tuple(3, 3));
  dims.push_back(make_tuple(4, 4));
  dims.push_back(make_tuple(5, 5));
  Action action(DROP);
  rules.push_back(new Rule(action, dims, "a", "")); // 2
  rules.push_back(new Rule(action, dims, "a", "")); // 3
  rules.push_back(new Rule("")); // 4
  parse::compute_relevant_sub_rulesets(rules, 2, domains);
  BOOST_CHECK_EQUAL(domains.size(), 1);
  BOOST_CHECK(domains[0] == make_tuple(2, 3));
  domains.clear();

  rules.push_back(new Rule(""));  // 5
  rules.push_back(new Rule(action, dims, "a", "")); // 6
  rules.push_back(new Rule(action, dims, "a", "")); // 7
  rules.push_back(new Rule(""));  // 8
  rules.push_back(new Rule(action, dims, "a", ""));  // 9
  rules.push_back(new Rule(""));  // 10
  rules.push_back(new Rule(action, dims, "a", "")); // 11
  rules.push_back(new Rule(action, dims, "a", "")); // 12
  rules.push_back(new Rule(action, dims, "a", "")); // 13
  parse::compute_relevant_sub_rulesets(rules, 2, domains);
  BOOST_CHECK_EQUAL(domains.size(), 3);
  BOOST_CHECK(domains[0] == make_tuple(2, 3));
  BOOST_CHECK(domains[1] == make_tuple(6, 7));
  BOOST_CHECK(domains[2] == make_tuple(11, 13));
  domains.clear();
  Rule::delete_rules(rules);
  rules.clear();

  rules.push_back(new Rule(action, dims, "a", ""));
  parse::compute_relevant_sub_rulesets(rules, 1, domains);
  BOOST_CHECK_EQUAL(domains.size(), 1);
  BOOST_CHECK(domains[0] == make_tuple(0, 0));

  Rule::delete_rules(rules);
  rules.clear();
  domains.clear();
  parse::compute_relevant_sub_rulesets(rules, 1, domains);
  BOOST_CHECK(domains.empty());

  rules.push_back(new Rule(action, dims, "a", ""));
  rules.push_back(new Rule(action, dims, "a", ""));
  rules.push_back(new Rule(action, dims, "a", ""));
  parse::compute_relevant_sub_rulesets(rules, 4, domains);
  BOOST_CHECK(domains.empty());
  parse::compute_relevant_sub_rulesets(rules, 3, domains);
  BOOST_CHECK_EQUAL(domains.size(), 1);
  BOOST_CHECK(domains[0] == make_tuple(0, 2));
  Rule::delete_rules(rules);
}


BOOST_AUTO_TEST_CASE(parse_group_rules_by_chain) {
  RuleVector rules;
  rules.push_back(parse::parse_rule("-A c1 -p udp -j DROP"));
  rules.push_back(parse::parse_rule("-A c2 -p udp -j DROP"));
  rules.push_back(parse::parse_rule("-A c2 -p udp -j DROP"));
  rules.push_back(parse::parse_rule("-A c3 -p udp -j DROP"));
  rules.push_back(parse::parse_rule("-A c3 -p udp -j DROP"));
  rules.push_back(parse::parse_rule("-A c3 -p udp -j DROP"));
  ChainVector chains;
  parse::group_rules_by_chain(rules, chains);
  BOOST_CHECK_EQUAL(chains.size(), 3);
  BOOST_CHECK_EQUAL(chains[0].size(), 1);
  BOOST_CHECK_EQUAL(chains[1].size(), 2);
  BOOST_CHECK_EQUAL(chains[2].size(), 3);
  Rule::delete_rules(rules);
}


BOOST_AUTO_TEST_CASE(parse_sort_by_protocol) {
  RuleVector rules;
  rules.push_back(parse::parse_rule("-A c -p tcp --sport 2"));
  rules.push_back(parse::parse_rule("-A c -p tcp --sport 1"));
  rules.push_back(parse::parse_rule("-A c -p udp --sport 3"));
  rules.push_back(parse::parse_rule("-A c -p tcp --sport 4"));
  rules.push_back(parse::parse_rule("-A c -p udp --sport 5"));
  rules.push_back(parse::parse_rule("-A c -p udp --sport 6"));
  rules.push_back(parse::parse_rule("-A c -p tcp --sport 7"));
  rules.push_back(parse::parse_rule("-A c -p tcp --sport 8"));
  parse::sort_by_protocol(rules);
  BOOST_CHECK_EQUAL(rules.size(), 8);
  BOOST_CHECK_EQUAL(rules[0]->src(), "-A c -p tcp --sport 2");
  BOOST_CHECK_EQUAL(rules[1]->src(), "-A c -p tcp --sport 1");
  BOOST_CHECK_EQUAL(rules[2]->src(), "-A c -p tcp --sport 4");
  BOOST_CHECK_EQUAL(rules[3]->src(), "-A c -p tcp --sport 7");
  BOOST_CHECK_EQUAL(rules[4]->src(), "-A c -p tcp --sport 8");
  BOOST_CHECK_EQUAL(rules[5]->src(), "-A c -p udp --sport 3");
  BOOST_CHECK_EQUAL(rules[6]->src(), "-A c -p udp --sport 5");
  BOOST_CHECK_EQUAL(rules[7]->src(), "-A c -p udp --sport 6");
  Rule::delete_rules(rules);
}

BOOST_AUTO_TEST_CASE(parse_compute_relevant_sub_rulesets_with_sort) {
  RuleVector rules;
  rules.push_back(parse::parse_rule("-A c -p tcp --sport 2"));
  rules.push_back(parse::parse_rule("-A c -p tcp --sport 1"));
  rules.push_back(parse::parse_rule("-A c -p udp --sport 3"));
  rules.push_back(parse::parse_rule("-A c -p tcp --sport 4"));
  rules.push_back(parse::parse_rule("-A c -p udp --sport 5"));
  rules.push_back(parse::parse_rule("-A c -p udp --sport 6"));
  rules.push_back(parse::parse_rule("-A c -p tcp --sport 7"));
  rules.push_back(parse::parse_rule("-A c -p tcp --sport 8"));
  DomainVector domains;
  parse::compute_relevant_sub_rulesets(rules, 3, domains);
  
  BOOST_CHECK_EQUAL(domains.size(), 2);
  BOOST_CHECK(domains[0] == make_tuple(0, 4));
  BOOST_CHECK(domains[1] == make_tuple(5, 7));

  BOOST_CHECK_EQUAL(rules.size(), 8);
  BOOST_CHECK_EQUAL(rules[0]->src(), "-A c -p tcp --sport 2");
  BOOST_CHECK_EQUAL(rules[1]->src(), "-A c -p tcp --sport 1");
  BOOST_CHECK_EQUAL(rules[2]->src(), "-A c -p tcp --sport 4");
  BOOST_CHECK_EQUAL(rules[3]->src(), "-A c -p tcp --sport 7");
  BOOST_CHECK_EQUAL(rules[4]->src(), "-A c -p tcp --sport 8");
  BOOST_CHECK_EQUAL(rules[5]->src(), "-A c -p udp --sport 3");
  BOOST_CHECK_EQUAL(rules[6]->src(), "-A c -p udp --sport 5");
  BOOST_CHECK_EQUAL(rules[7]->src(), "-A c -p udp --sport 6");
  Rule::delete_rules(rules);
}

/*****************************************************************************
 *                            A R G   T E S T S                              *
 *****************************************************************************/

BOOST_AUTO_TEST_CASE(arg_parse_binth) {
  Arguments args;
  BOOST_CHECK_EQUAL(args.binth(), 4);

  StrVector correct;
  correct.push_back("17");
  correct.push_back("000000000000000001");
  correct.push_back("65536");
  correct.push_back("00000065536");
  correct.push_back("1");
  for (size_t i = 0; i < correct.size(); ++i) {
    stringstream ss(correct[i]);
    size_t expected;
    ss >> expected;
    args.parse_binth(correct[i]);
    BOOST_CHECK_EQUAL(args.binth(), expected);
  }

  StrVector fails;
  fails.push_back("adsad");
  fails.push_back("100000000");
  fails.push_back("00000");
  fails.push_back("0");
  for (size_t i = 0; i < fails.size(); ++i) {
    bool thrown = false;
    try {
      args.parse_binth(fails[i]);
    } catch (const string& msg) {
      stringstream ss;
      ss << "Invalid parameter --binth ('" << fails[i] << "'): "
          << "must be an integer between 1 and 65536!";
      BOOST_CHECK(msg == ss.str());
      thrown = true;
    }
    BOOST_CHECK(thrown);
  }
}


BOOST_AUTO_TEST_CASE(arg_parse_spfac) {
  Arguments args;
  BOOST_CHECK_EQUAL(args.spfac(), 4);

  StrVector correct;
  correct.push_back("17");
  correct.push_back("000000000000000001");
  correct.push_back("65536");
  correct.push_back("00000065536");
  correct.push_back("1");
  for (size_t i = 0; i < correct.size(); ++i) {
    stringstream ss(correct[i]);
    size_t expected;
    ss >> expected;
    args.parse_spfac(correct[i]);
    BOOST_CHECK_EQUAL(args.spfac(), expected);
  }

  StrVector fails;
  fails.push_back("adsad");
  fails.push_back("100000000");
  fails.push_back("00000");
  fails.push_back("0");
  for (size_t i = 0; i < fails.size(); ++i) {
    bool thrown = false;
    try {
      args.parse_spfac(fails[i]);
    } catch (const string& msg) {
      stringstream ss;
      ss << "Invalid parameter --spfac ('" << fails[i] << "'): "
          << "must be an integer between 1 and 65536!";
      BOOST_CHECK(msg == ss.str());
      thrown = true;
    }
    BOOST_CHECK(thrown);
  }
}


BOOST_AUTO_TEST_CASE(arg_parse_dim_choice) {
  Arguments args;
  BOOST_CHECK_EQUAL(args.dim_choice(), Arguments::DIM_CHOICE_MAX_DISTINCT);
  args.parse_dim_choice("least-max");
  BOOST_CHECK_EQUAL(args.dim_choice(), Arguments::DIM_CHOICE_LEAST_MAX_RULES);
  args.parse_dim_choice("max-dist");
  BOOST_CHECK_EQUAL(args.dim_choice(), Arguments::DIM_CHOICE_MAX_DISTINCT);
  
  bool thrown = false;
  try {
    args.parse_dim_choice("xxx");
  } catch (const string& msg) {
    stringstream ss;
    ss << "Invalid parameter --dim-choice ('xxx'):";
    ss << " must be 'max-dist' or 'least-max'!";
    BOOST_CHECK(msg == ss.str());
    thrown = true;
  }
  BOOST_CHECK(thrown);
}


BOOST_AUTO_TEST_CASE(arg_parse_search) {
  Arguments args;
  BOOST_CHECK_EQUAL(args.search(), Arguments::SEARCH_LINEAR);
  args.parse_search("binary");
  BOOST_CHECK_EQUAL(args.search(), Arguments::SEARCH_BINARY);
  args.parse_search("linear");
  BOOST_CHECK_EQUAL(args.search(), Arguments::SEARCH_LINEAR);
  
  bool thrown = false;
  try {
    args.parse_search("xxx");
  } catch (const string& msg) {
    stringstream ss;
    ss << "Invalid parameter --search ('xxx'):";
    ss << " must be 'linear' or 'binary'!";
    BOOST_CHECK(msg == ss.str());
    thrown = true;
  }
  BOOST_CHECK(thrown);
}


BOOST_AUTO_TEST_CASE(arg_parse_min_rules) {
  Arguments args;
  BOOST_CHECK_EQUAL(args.min_rules(), 10);
  args.parse_min_rules("20");
  BOOST_CHECK_EQUAL(args.min_rules(), 20);

  StrVector fails;
  fails.push_back("bla");
  fails.push_back("0");
  fails.push_back("-1");
  for (size_t i = 0; i < fails.size(); ++i) {
    const string& fail = fails[i];
    bool thrown = false;
    try {
      args.parse_min_rules(fail);
    } catch (const string& msg) {
      thrown = true;
      stringstream ss;
      ss << "Invalid parameter --min-rules ('" << fail << "'):";
      ss << " must be a positive integer!";
      BOOST_CHECK(ss.str() == msg);
    }
    BOOST_CHECK(thrown);
  }
}


BOOST_AUTO_TEST_CASE(arg_parse_arg_vector) {
  StrVector vector;
  vector.push_back("--binth");
  vector.push_back("5");
  vector.push_back("--search");
  vector.push_back("binary");
  vector.push_back("--spfac");
  vector.push_back("20");
  vector.push_back("--dim-choice");
  vector.push_back("least-max");
  vector.push_back("--infile");
  vector.push_back("FILENAME");

  Arguments a1(Arguments::parse_arg_vector(vector));
  BOOST_CHECK_EQUAL(a1.binth(), 5);
  BOOST_CHECK_EQUAL(a1.spfac(), 20);
  BOOST_CHECK_EQUAL(a1.search(), Arguments::SEARCH_BINARY);
  BOOST_CHECK_EQUAL(a1.dim_choice(), Arguments::DIM_CHOICE_LEAST_MAX_RULES);
  BOOST_CHECK(a1.infile() == "FILENAME");

  vector.clear();
  bool thrown = false;
  try {
    Arguments a2(Arguments::parse_arg_vector(vector));
  } catch (const string& msg) {
    thrown = true;
  }
  BOOST_CHECK(thrown);
}


BOOST_AUTO_TEST_CASE(arg_parse_arg_vector_usage) {
  StrVector v;
  v.push_back("--usage");
  bool thrown = false;
  try {
    Arguments::parse_arg_vector(v);
  } catch (const string& msg) {
    BOOST_CHECK(msg == "usage");
    thrown = true;
  }
  BOOST_CHECK(thrown);
}


BOOST_AUTO_TEST_CASE(arg_parse_arg_vector_verbose) {
  StrVector v;
  v.push_back("--verbose");
  v.push_back("--infile");
  v.push_back("blabla");
  Arguments args(Arguments::parse_arg_vector(v));
  BOOST_CHECK(args.verbose());

  v.erase(v.begin());
  args = Arguments::parse_arg_vector(v);
  BOOST_CHECK(!args.verbose());
}

/*****************************************************************************
 *                        E M I T T E R   T E S T S                          *
 *****************************************************************************/

BOOST_AUTO_TEST_CASE(emit_emit_non_applicable_rule) {
  Emitter e(NodeVector(), RuleVector(), DomainVector(), 0);
  stringstream ss;
  Rule* rule = new Rule("abc");
  e.emit_non_applicable_rule(rule, ss);
  delete rule;
  BOOST_CHECK_EQUAL(ss.str(), "abc\n");
}


BOOST_AUTO_TEST_CASE(emit_emit_prefix) {
  Emitter e(NodeVector(), RuleVector(), DomainVector(), 0);
  stringstream ss, out;
  ss << "*filter\n" << ":INPUT ACCEPT [0:0]\n" << ":FORWARD ACCEPT [0:0]\n"
      << ":OUTPUT ACCEPT [0:0]\n";
  e.emit_prefix(out);
  BOOST_CHECK_EQUAL(ss.str(), out.str());
}


BOOST_AUTO_TEST_CASE(emit_emit_suffix) {
  Emitter e(NodeVector(), RuleVector(), DomainVector(), 0);
  stringstream out;
  e.emit_suffix(out);
  BOOST_CHECK_EQUAL("COMMIT\n", out.str());
}


BOOST_AUTO_TEST_CASE(emit_emit_leaf) {
  RuleVector rules;
  rules.push_back(parse::parse_rule("-A CHAIN -p tcp --sport 1 -j DROP"));
  rules.push_back(parse::parse_rule("-A CHAIN -p tcp --sport 1 -j DROP"));
  rules.push_back(parse::parse_rule("-A CHAIN -p tcp --sport 1 -j DROP"));
  DomainTuple domain(make_tuple(0, 2));
  TreeNode tree(rules, domain);
  Emitter emitter(NodeVector(), RuleVector(), DomainVector(),
      Arguments::SEARCH_LINEAR);
  stringstream out;
  emitter.emit_leaf(&tree, "CURRENT_CHAIN", "NEXT_CHAIN", out);

  stringstream expect;
  expect << "# leaf node" << endl
      << "-A CURRENT_CHAIN -p tcp --sport 1 -j DROP" << endl 
      << "-A CURRENT_CHAIN -p tcp --sport 1 -j DROP" << endl
      << "-A CURRENT_CHAIN -p tcp --sport 1 -j DROP" << endl
      << "-A CURRENT_CHAIN -j NEXT_CHAIN" << endl << endl;
  BOOST_CHECK_EQUAL(out.str(), expect.str());
  Rule::delete_rules(rules);
}


BOOST_AUTO_TEST_CASE(emit_num_to_ip) {
  BOOST_CHECK_EQUAL(Emitter::num_to_ip(0), "0.0.0.0");
  BOOST_CHECK_EQUAL(Emitter::num_to_ip(1), "0.0.0.1");
  BOOST_CHECK_EQUAL(Emitter::num_to_ip(256), "0.0.1.0");
  BOOST_CHECK_EQUAL(Emitter::num_to_ip(16909060), "1.2.3.4");
  BOOST_CHECK_EQUAL(Emitter::num_to_ip(4294967295), "255.255.255.255");
}

/*****************************************************************************
 *                           R U L E   T E S T S                             *
 *****************************************************************************/

BOOST_AUTO_TEST_CASE(rule_src_with_patched_chain) {
  Rule* rule = parse::parse_rule("-A abc -p tcp -j DROP");
  BOOST_CHECK_EQUAL(rule->src_with_patched_chain("blablub"),
      std::string("-A blablub -p tcp -j DROP"));
  delete rule;
}
