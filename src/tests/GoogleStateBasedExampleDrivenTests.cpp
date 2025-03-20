#include <gtest/gtest.h>

#include <utility>

#include "RBtreeBuilder.hpp"
#include "RBtreeValidator.hpp"
#include "RBtreeVizualizer.hpp"

using tree_type = RBtree<int, int>;
using validator_type = decltype(RBtreeValidator(std::declval<tree_type&>()));
using builder_type = decltype(RBtreeBuilder(std::declval<tree_type&>()));
using color_type = builder_type::color_type;
using direction_type = builder_type::direction_type;

TEST(INSERT, EdgeCase_InsertToEmptyTree) {
  tree_type tree;

  tree.emplace(42, 0);

  validator_type validator(tree);
  ASSERT_TRUE(validator.validate()) << validator;

  tree_type expected_tree;
  builder_type expected_builder(expected_tree);
  expected_builder.root(42, color_type::Black).finalize();

  ASSERT_EQ(tree, expected_tree);
}

TEST(INSERT, Case1_RedUncle_RecolorRequired) {
  tree_type tree;
  builder_type builder(tree);

  builder.root(50, color_type::Black)
      .add_node(30, color_type::Red, 50, direction_type::Left)
      .add_node(70, color_type::Red, 50, direction_type::Right)
      .finalize();

  tree.emplace(20, 0);

  validator_type validator(tree);
  ASSERT_TRUE(validator.validate()) << validator;

  tree_type expected_tree;
  builder_type expected_builder(expected_tree);
  expected_builder.root(50, color_type::Black)
      .add_node(30, color_type::Black, 50, direction_type::Left)
      .add_node(70, color_type::Black, 50, direction_type::Right)
      .add_node(20, color_type::Red, 30, direction_type::Left)
      .finalize();

  ASSERT_EQ(tree, expected_tree);
}

TEST(INSERT, Case2_BlackUncle_LeftLeftRotation) {
  tree_type tree;
  builder_type builder(tree);

  builder.root(50, color_type::Black)
      .add_node(30, color_type::Red, 50, direction_type::Left)
      .finalize();

  tree.emplace(20, 0);

  validator_type validator(tree);
  ASSERT_TRUE(validator.validate()) << validator;

  tree_type expected_tree;
  builder_type expected_builder(expected_tree);
  expected_builder.root(30, color_type::Black)
      .add_node(20, color_type::Red, 30, direction_type::Left)
      .add_node(50, color_type::Red, 30, direction_type::Right)
      .finalize();

  ASSERT_EQ(tree, expected_tree);
}

TEST(INSERT, Case3_BlackUncle_LeftRightRotation) {
  tree_type tree;
  builder_type builder(tree);

  builder.root(50, color_type::Black)
      .add_node(30, color_type::Red, 50, direction_type::Left)
      .finalize();

  tree.emplace(40, 0);

  validator_type validator(tree);
  ASSERT_TRUE(validator.validate()) << validator;

  tree_type expected_tree;
  builder_type expected_builder(expected_tree);
  expected_builder.root(40, color_type::Black)
      .add_node(30, color_type::Red, 40, direction_type::Left)
      .add_node(50, color_type::Red, 40, direction_type::Right)
      .finalize();

  ASSERT_EQ(tree, expected_tree);
}

TEST(ERASE, EdgeCase_EraseLastElement) {
  tree_type tree;
  builder_type builder(tree);

  builder.root(0, color_type::Black).finalize();

  tree.erase(0);

  validator_type validator(tree);
  ASSERT_TRUE(validator.validate()) << validator;

  tree_type expected_tree;

  ASSERT_EQ(tree, expected_tree);
}

TEST(ERASE, Case1_SiblingIsRed) {
  tree_type tree;
  builder_type builder(tree);

  builder.root(50, color_type::Black)
      .add_node(30, color_type::Red, 50, direction_type::Left)
      .add_node(70, color_type::Black, 50, direction_type::Right)
      .add_node(20, color_type::Black, 30, direction_type::Left)
      .add_node(40, color_type::Black, 30, direction_type::Right)
      .finalize();

  tree.erase(20);

  validator_type validator(tree);
  ASSERT_TRUE(validator.validate()) << validator;

  tree_type expected_tree;
  builder_type expected_builder(expected_tree);
  expected_builder.root(50, color_type::Black)
      .add_node(30, color_type::Black, 50, direction_type::Left)
      .add_node(70, color_type::Black, 50, direction_type::Right)
      .add_node(40, color_type::Red, 30, direction_type::Right)
      .finalize();

  ASSERT_EQ(tree, expected_tree);
}

TEST(ERASE, Case2_SiblingIsBlackWithBlackChildren) {
  tree_type tree;
  builder_type builder(tree);
  builder.root(50, color_type::Black)
      .add_node(30, color_type::Black, 50, direction_type::Left)
      .add_node(70, color_type::Black, 50, direction_type::Right)
      .add_node(20, color_type::Black, 30, direction_type::Left)
      .add_node(40, color_type::Black, 30, direction_type::Right)
      .add_node(55, color_type::Black, 70, direction_type::Left)
      .add_node(75, color_type::Black, 70, direction_type::Right)
      .finalize();

  tree.erase(20);

  validator_type validator(tree);
  ASSERT_TRUE(validator.validate()) << validator;

  tree_type expected_tree;
  builder_type expected_builder(expected_tree);
  expected_builder.root(50, color_type::Black)
      .add_node(30, color_type::Black, 50, direction_type::Left)
      .add_node(70, color_type::Red, 50, direction_type::Right)
      .add_node(40, color_type::Red, 30, direction_type::Right)
      .add_node(55, color_type::Black, 70, direction_type::Left)
      .add_node(75, color_type::Black, 70, direction_type::Right)
      .finalize();

  ASSERT_EQ(tree, expected_tree);
}

TEST(ERASE, Case3_SiblingIsBlackWithRedLeftChild) {
  tree_type tree;
  builder_type builder(tree);

  builder.root(50, color_type::Black)
      .add_node(30, color_type::Black, 50, direction_type::Left)
      .add_node(70, color_type::Black, 50, direction_type::Right)
      .add_node(60, color_type::Red, 70, direction_type::Left)
      .add_node(80, color_type::Black, 70, direction_type::Right)
      .add_node(25, color_type::Black, 30, direction_type::Left)
      .add_node(35, color_type::Black, 30, direction_type::Right)
      .add_node(55, color_type::Black, 60, direction_type::Left)
      .add_node(65, color_type::Black, 60, direction_type::Right)
      .finalize();

  tree.erase(30);

  validator_type validator(tree);
  ASSERT_TRUE(validator.validate()) << validator;

  tree_type expected_tree;
  builder_type expected_builder(expected_tree);
  expected_builder.root(60, color_type::Black)
      .add_node(50, color_type::Black, 60, direction_type::Left)
      .add_node(70, color_type::Black, 60, direction_type::Right)
      .add_node(35, color_type::Black, 50, direction_type::Left)
      .add_node(55, color_type::Black, 50, direction_type::Right)
      .add_node(65, color_type::Black, 70, direction_type::Left)
      .add_node(80, color_type::Black, 70, direction_type::Right)
      .add_node(25, color_type::Red, 35, direction_type::Left)
      .finalize();

  ASSERT_EQ(tree, expected_tree);
}

TEST(ERASE, Case4_SiblingIsBlackWithRedRightChild) {
  tree_type tree;
  builder_type builder(tree);

  builder.root(50, color_type::Black)
      .add_node(30, color_type::Black, 50, direction_type::Left)
      .add_node(70, color_type::Black, 50, direction_type::Right)
      .add_node(60, color_type::Black, 70, direction_type::Left)
      .add_node(80, color_type::Red, 70, direction_type::Right)
      .add_node(75, color_type::Black, 80, direction_type::Left)
      .add_node(85, color_type::Black, 80, direction_type::Right)
      .add_node(25, color_type::Black, 30, direction_type::Left)
      .add_node(35, color_type::Black, 30, direction_type::Right)
      .finalize();

  tree.erase(30);

  validator_type validator(tree);
  ASSERT_TRUE(validator.validate()) << validator;

  tree_type expected_tree;
  builder_type expected_builder(expected_tree);
  expected_builder.root(70, color_type::Black)
      .add_node(50, color_type::Black, 70, direction_type::Left)
      .add_node(80, color_type::Black, 70, direction_type::Right)
      .add_node(35, color_type::Black, 50, direction_type::Left)
      .add_node(60, color_type::Black, 50, direction_type::Right)
      .add_node(25, color_type::Red, 35, direction_type::Left)
      .add_node(75, color_type::Black, 80, direction_type::Left)
      .add_node(85, color_type::Black, 80, direction_type::Right)
      .finalize();

  ASSERT_EQ(tree, expected_tree);
}

TEST(ERASE, Case5_NodeIsLeftChildAndSiblingHasRedRightChild) {
  tree_type tree;
  builder_type builder(tree);

  builder.root(50, color_type::Black)
      .add_node(30, color_type::Black, 50, direction_type::Left)
      .add_node(70, color_type::Black, 50, direction_type::Right)
      .add_node(80, color_type::Red, 70, direction_type::Right)
      .finalize();

  tree.erase(30);

  validator_type validator(tree);
  ASSERT_TRUE(validator.validate()) << validator;

  tree_type expected_tree;
  builder_type expected_builder(expected_tree);
  expected_builder.root(70, color_type::Black)
      .add_node(50, color_type::Black, 70, direction_type::Left)
      .add_node(80, color_type::Black, 70, direction_type::Right)
      .finalize();

  ASSERT_EQ(tree, expected_tree);
}

TEST(ERASE, Case6_NodeIsRightChildAndSiblingHasRedLeftChild) {
  tree_type tree;
  builder_type builder(tree);

  builder.root(50, color_type::Black)
      .add_node(70, color_type::Black, 50, direction_type::Right)
      .add_node(30, color_type::Black, 50, direction_type::Left)
      .add_node(20, color_type::Red, 30, direction_type::Left)
      .finalize();

  tree.erase(30);

  validator_type validator(tree);
  ASSERT_TRUE(validator.validate()) << validator;

  tree_type expected_tree;
  builder_type expected_builder(expected_tree);
  expected_builder.root(50, color_type::Black)
      .add_node(20, color_type::Black, 50, direction_type::Left)
      .add_node(70, color_type::Black, 50, direction_type::Right)
      .finalize();

  ASSERT_EQ(tree, expected_tree);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
