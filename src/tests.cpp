#include "box.hpp"
#include "treenode.hpp"
#include "parse.hpp"
#include <cstdio>

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
  Box rule1(rule1_bounds);                        \
                                                  \
  DimVector rule2_bounds;                         \
  rule2_bounds.push_back(make_tuple(4, 7));       \
  Box rule2(rule2_bounds);                        \
                                                  \
  DimVector rule3_bounds;                         \
  rule3_bounds.push_back(make_tuple(8, 10));      \
  Box rule3(rule3_bounds);                        \
                                                  \
  node.add_rule(static_cast<const Box*>(&rule1)); \
  node.add_rule(static_cast<const Box*>(&rule2)); \
  node.add_rule(static_cast<const Box*>(&rule3));


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
  std::vector<Box*> rule_pointers;
  for (size_t i = 0; i < 10; ++i) {
    DimVector rule_bounds;
    rule_bounds.push_back(make_tuple(i, i));
    Box* rule = new Box(rule_bounds);
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
  std::vector<Box*> rule_pointers;
  for (size_t i = 0; i < 10; ++i) {
    DimVector rule_bounds;
    rule_bounds.push_back(make_tuple(i, i));
    Box* rule = new Box(rule_bounds);
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
  Box rule1(rule1_bounds);
  node.add_rule(&rule1);

  DimVector rule2_bounds;
  rule2_bounds.push_back(make_tuple(6, 9));
  rule2_bounds.push_back(make_tuple(0, 1));
  Box rule2(rule2_bounds);
  node.add_rule(&rule2);

  const size_t dim = node.dim_max_distinct_rules();
  BOOST_CHECK_EQUAL(dim, 0);

  TreeNode node2(bounds);

  DimVector rule3_bounds;
  rule3_bounds.push_back(make_tuple(0, 1));
  rule3_bounds.push_back(make_tuple(1, 5));
  Box rule3(rule3_bounds);
  node2.add_rule(&rule3);

  DimVector rule4_bounds;
  rule4_bounds.push_back(make_tuple(0, 1));
  rule4_bounds.push_back(make_tuple(6, 9));
  Box rule4(rule4_bounds);
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
  Box rule1(rule1_bounds);
  node.add_rule(&rule1);

  DimVector rule2_bounds;
  rule2_bounds.push_back(make_tuple(0, 10));
  rule2_bounds.push_back(make_tuple(8, 8));
  Box rule2(rule2_bounds);
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

BOOST_AUTO_TEST_CASE(parse_parse_ruleset_simple) {
  StrVector lines;
  lines.push_back("*filter");
  lines.push_back(":INPUT ACCEPT [0:0]");
  lines.push_back(":FORWARD ACCEPT [0:0]");
  lines.push_back(":OUTPUT ACCEPT [0:0]");
  lines.push_back("COMMIT");
  RuleVector rules;
  BOOST_CHECK_EQUAL(parse::parse_ruleset(lines, rules), 0);
  BOOST_CHECK(rules.empty());
}

BOOST_AUTO_TEST_CASE(parse_parse_ip) {
  BOOST_CHECK_EQUAL(parse::parse_ip("1.2.3.4"), 16909060);
  BOOST_CHECK_EQUAL(parse::parse_ip("0.00.000.0"), 0);
  BOOST_CHECK_EQUAL(parse::parse_ip("255.255.255.255"), 4294967295);
  BOOST_CHECK_EQUAL(parse::parse_ip("asdasd"), -1);
}
