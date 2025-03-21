#include <gtest/gtest.h>

#include <memory>

#include "AuxiliaryAllocators.hpp"
#include "RBtree.hpp"

using basic_key_type = int;
using basic_T_type = int;
using basic_compare_type = std::less<basic_key_type>;
using basic_allocator_type =
    std::allocator<std::pair<const basic_key_type, basic_T_type>>;
using basic_value_type = std::pair<const basic_key_type, basic_T_type>;

template <typename Key, typename T, typename Compare = std::less<Key>,
          typename Allocator = std::allocator<std::pair<const Key, T>>>
using tree_type = RBtree<Key, T, Compare, Allocator>;

template <bool IsPropagated>

using allocator_swap_policy_specified =
    PolicyAwareAllocator<basic_value_type, true, false, false, false,
                         IsPropagated>;

template <bool IsPropagated>
using allocator_copy_construction_policy_specified =
    PolicyAwareAllocator<basic_value_type, true, IsPropagated, false, false,
                         true>;

template <bool IsAlwaysEqual, bool IsPropagated>
using allocator_copy_policy_specified =
    PolicyAwareAllocator<basic_value_type, IsAlwaysEqual, false, IsPropagated,
                         false, true>;

template <bool IsAlwaysEqual, bool IsPropagated>
using allocator_move_policy_specified =
    PolicyAwareAllocator<basic_value_type, IsAlwaysEqual, false, false,
                         IsPropagated, true>;

template <typename Allocator>
using fixed_allocator_tree_type =
    tree_type<basic_key_type, basic_T_type, basic_compare_type, Allocator>;

TEST(RBtreeAllocatorTest, SwapPropagatedFalse) {
  auto allocator_1 = allocator_swap_policy_specified<false>(1);
  auto allocator_2 = allocator_swap_policy_specified<false>(2);

  fixed_allocator_tree_type<decltype(allocator_1)> tree_1(allocator_1);
  fixed_allocator_tree_type<decltype(allocator_2)> tree_2(allocator_2);

  std::swap(tree_1, tree_2);

  ASSERT_EQ(tree_1.get_allocator().id, 1);
  ASSERT_EQ(tree_2.get_allocator().id, 2);
}

TEST(RBtreeAllocatorTest, SwapPropagatedTrue) {
  auto allocator_1 = allocator_swap_policy_specified<true>(1);
  auto allocator_2 = allocator_swap_policy_specified<true>(2);

  fixed_allocator_tree_type<decltype(allocator_1)> tree_1(allocator_1);
  fixed_allocator_tree_type<decltype(allocator_2)> tree_2(allocator_2);

  std::swap(tree_1, tree_2);

  ASSERT_EQ(tree_1.get_allocator().id, 2);
  ASSERT_EQ(tree_2.get_allocator().id, 1);
}

TEST(RBtreeAllocatorTest, SelectOnCopyConstructionFalse) {
  auto allocator = allocator_copy_construction_policy_specified<false>(1);

  fixed_allocator_tree_type<decltype(allocator)> tree_1(allocator);
  decltype(tree_1) tree_2(tree_1);

  ASSERT_EQ(tree_1.get_allocator().id, 1);
  ASSERT_EQ(tree_2.get_allocator().id, decltype(allocator)::kDefaultId);
}

TEST(RBtreeAllocatorTest, SelectOnCopyConstructionTrue) {
  auto allocator = allocator_copy_construction_policy_specified<true>(1);

  fixed_allocator_tree_type<decltype(allocator)> tree_1(allocator);
  decltype(tree_1) tree_2(tree_1);

  ASSERT_EQ(tree_1.get_allocator().id, 1);
  ASSERT_EQ(tree_2.get_allocator().id, 1);
}

TEST(RBtreeAllocatorTest, PropagateOnCopyAssignmentTrue) {
  auto allocator = allocator_copy_policy_specified<false, true>(1);

  fixed_allocator_tree_type<decltype(allocator)> tree_1(allocator);
  decltype(tree_1) tree_2;
  tree_2 = tree_1;

  ASSERT_EQ(tree_1.get_allocator().id, 1);
  ASSERT_EQ(tree_2.get_allocator().id, 1);
}

TEST(RBtreeAllocatorTest, IsAlwaysEqualTrueAndPropagateOnCopyAssignmentFalse) {
  auto allocator_1 = allocator_copy_policy_specified<true, false>(1);
  auto allocator_2 = allocator_copy_policy_specified<true, false>(2);

  fixed_allocator_tree_type<decltype(allocator_1)> tree_1(allocator_1);
  fixed_allocator_tree_type<decltype(allocator_2)> tree_2(allocator_2);
  tree_2 = tree_1;

  ASSERT_EQ(tree_1.get_allocator().id, 1);
  ASSERT_EQ(tree_2.get_allocator().id, 2);
}

TEST(RBtreeAllocatorTest,
     IsAlwaysEqualFalseAndPropagateOnCopyAssignmentFalseAndAllocatorsEqual) {
  auto allocator_1 = allocator_copy_policy_specified<false, false>(1);
  auto allocator_2 = allocator_copy_policy_specified<false, false>(1);

  fixed_allocator_tree_type<decltype(allocator_1)> tree_1(allocator_1);
  fixed_allocator_tree_type<decltype(allocator_2)> tree_2(allocator_2);
  tree_2 = tree_1;

  ASSERT_EQ(tree_1.get_allocator().id, 1);
  ASSERT_EQ(tree_2.get_allocator().id, 1);
}

TEST(RBtreeAllocatorTest,
     IsAlwaysEqualFalseAndPropagateOnCopyAssignmentFalseAndAllocatorsNotEqual) {
  auto allocator_1 = allocator_copy_policy_specified<false, false>(1);
  auto allocator_2 = allocator_copy_policy_specified<false, false>(2);

  fixed_allocator_tree_type<decltype(allocator_1)> tree_1(allocator_1);
  fixed_allocator_tree_type<decltype(allocator_2)> tree_2(allocator_2);
  tree_2 = tree_1;

  ASSERT_EQ(tree_1.get_allocator().id, 1);
  ASSERT_EQ(tree_2.get_allocator().id, 2);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}