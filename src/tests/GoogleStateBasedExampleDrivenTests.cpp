#include <gtest/gtest.h>

#include "RBtreeBuilder.hpp"
#include "RBtreeValidator.hpp"
#include "RBtreeVizualizer.hpp"

/*In fact, it doesn't matter where we get this type from.*/
using color_type =
    RBtreeBuilder<int, int, std::less<int>,
                  std::allocator<std::pair<const int, int>>>::color_type;

using direction_type =
    RBtreeBuilder<int, int, std::less<int>,
                  std::allocator<std::pair<const int, int>>>::direction_type;

TEST(INSERT, EdgeCase_InsertToEmptyTree) {
  RBtree<int, int> tree;

  tree.emplace(42, 0);

  RBtreeValidator validator(tree);
  ASSERT_TRUE(validator.validate());

  RBtree<int, int> expected_tree;
  RBtreeBuilder expected_builder(expected_tree);
  expected_builder.root(42, color_type::Black).finalize();

  ASSERT_EQ(tree, expected_tree);
}

TEST(INSERT, Case1_RedUncle_RecolorRequired) {
  RBtree<int, int> tree;
  RBtreeBuilder builder(tree);

  builder.root(50, color_type::Black)
      .add_node(30, color_type::Red, 50, direction_type::Left)
      .add_node(70, color_type::Red, 50, direction_type::Right)
      .finalize();

  tree.emplace(20, 0);

  RBtreeValidator validator(tree);
  ASSERT_TRUE(validator.validate());

  RBtree<int, int> expected_tree;
  RBtreeBuilder expected_builder(expected_tree);
  expected_builder.root(50, color_type::Black)
      .add_node(30, color_type::Black, 50, direction_type::Left)
      .add_node(70, color_type::Black, 50, direction_type::Right)
      .add_node(20, color_type::Red, 30, direction_type::Left)
      .finalize();

  ASSERT_EQ(tree, expected_tree);
}

TEST(INSERT, Case2_BlackUncle_LeftLeftRotation) {
  RBtree<int, int> tree;
  RBtreeBuilder builder(tree);

  builder.root(50, color_type::Black)
      .add_node(30, color_type::Red, 50, direction_type::Left)
      .finalize();

  tree.emplace(20, 0);

  RBtreeValidator validator(tree);
  ASSERT_TRUE(validator.validate());

  RBtree<int, int> expected_tree;
  RBtreeBuilder expected_builder(expected_tree);
  expected_builder.root(30, color_type::Black)
      .add_node(20, color_type::Red, 30, direction_type::Left)
      .add_node(50, color_type::Red, 30, direction_type::Right)
      .finalize();

  ASSERT_EQ(tree, expected_tree);
}

TEST(INSERT, Case3_BlackUncle_LeftRightRotation) {
  RBtree<int, int> tree;
  RBtreeBuilder builder(tree);

  builder.root(50, color_type::Black)
      .add_node(30, color_type::Red, 50, direction_type::Left)
      .finalize();

  tree.emplace(40, 0);

  RBtreeValidator validator(tree);
  ASSERT_TRUE(validator.validate());

  RBtree<int, int> expected_tree;
  RBtreeBuilder expected_builder(expected_tree);
  expected_builder.root(40, color_type::Black)
      .add_node(30, color_type::Red, 40, direction_type::Left)
      .add_node(50, color_type::Red, 40, direction_type::Right)
      .finalize();

  ASSERT_EQ(tree, expected_tree);
}

TEST(ERASE, EdgeCase_EraseLastElement) {
  RBtree<int, int> tree;
  RBtreeBuilder builder(tree);

  builder.root(0, color_type::Black).finalize();

  tree.erase(0);

  RBtreeValidator validator(tree);
  ASSERT_TRUE(validator.validate());

  RBtree<int, int> expected_tree;

  ASSERT_EQ(tree, expected_tree);
}

TEST(ERASE, Case1_SiblingIsRed) {
  RBtree<int, int> tree;
  RBtreeBuilder builder(tree);

  builder.root(50, color_type::Black)
      .add_node(30, color_type::Red, 50, direction_type::Left)
      .add_node(70, color_type::Black, 50, direction_type::Right)
      .add_node(20, color_type::Black, 30, direction_type::Left)
      .add_node(40, color_type::Black, 30, direction_type::Right)
      .finalize();

  tree.erase(20);

  RBtreeValidator validator(tree);
  ASSERT_TRUE(validator.validate());

  RBtree<int, int> expected_tree;
  RBtreeBuilder expected_builder(expected_tree);
  expected_builder.root(50, color_type::Black)
      .add_node(30, color_type::Black, 50, direction_type::Left)
      .add_node(70, color_type::Black, 50, direction_type::Right)
      .add_node(40, color_type::Red, 30, direction_type::Right)
      .finalize();

  ASSERT_EQ(tree, expected_tree);
}

TEST(ERASE, Case2_SiblingIsBlackWithBlackChildren) {
  RBtree<int, int> tree;
  RBtreeBuilder builder(tree);
  builder.root(50, color_type::Black)
      .add_node(30, color_type::Black, 50, direction_type::Left)
      .add_node(70, color_type::Black, 50, direction_type::Right)
      .add_node(20, color_type::Black, 30, direction_type::Left)
      .add_node(40, color_type::Black, 30, direction_type::Right)
      .add_node(55, color_type::Black, 70, direction_type::Left)
      .add_node(75, color_type::Black, 70, direction_type::Right)
      .finalize();

  tree.erase(20);

  RBtreeValidator validator(tree);
  ASSERT_TRUE(validator.validate());

  RBtree<int, int> expected_tree;
  RBtreeBuilder expected_builder(expected_tree);
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
  RBtree<int, int> tree;
  RBtreeBuilder builder(tree);

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

  RBtreeValidator validator(tree);
  ASSERT_TRUE(validator.validate());

  RBtree<int, int> expected_tree;
  RBtreeBuilder expected_builder(expected_tree);
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
  RBtree<int, int> tree;
  RBtreeBuilder builder(tree);

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

  RBtreeValidator validator(tree);
  ASSERT_TRUE(validator.validate());

  RBtree<int, int> expected_tree;
  RBtreeBuilder expected_builder(expected_tree);
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
  RBtree<int, int> tree;
  RBtreeBuilder builder(tree);

  builder.root(50, color_type::Black)
      .add_node(30, color_type::Black, 50, direction_type::Left)
      .add_node(70, color_type::Black, 50, direction_type::Right)
      .add_node(80, color_type::Red, 70, direction_type::Right)
      .finalize();

  tree.erase(30);

  RBtreeValidator validator(tree);
  ASSERT_TRUE(validator.validate());

  RBtree<int, int> expected_tree;
  RBtreeBuilder expected_builder(expected_tree);
  expected_builder.root(70, color_type::Black)
      .add_node(50, color_type::Black, 70, direction_type::Left)
      .add_node(80, color_type::Black, 70, direction_type::Right)
      .finalize();

  ASSERT_EQ(tree, expected_tree);
}

TEST(ERASE, Case6_NodeIsRightChildAndSiblingHasRedLeftChild) {
  RBtree<int, int> tree;
  RBtreeBuilder builder(tree);

  builder.root(50, color_type::Black)
      .add_node(70, color_type::Black, 50, direction_type::Right)
      .add_node(30, color_type::Black, 50, direction_type::Left)
      .add_node(20, color_type::Red, 30, direction_type::Left)
      .finalize();

  tree.erase(30);

  RBtreeValidator validator(tree);
  ASSERT_TRUE(validator.validate());

  RBtree<int, int> expected_tree;
  RBtreeBuilder expected_builder(expected_tree);
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
