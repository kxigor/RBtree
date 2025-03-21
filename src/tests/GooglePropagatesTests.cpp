#include <gtest/gtest.h>

#include "AuxiliaryAllocators.hpp"
#include "RBtree.hpp"

using tree_type = RBtree<int, int>;

template <bool IsPropagated>
using allocator_swap_policy_specified =
    PolicyAwareAllocator<tree_type::value_type, true, false, false, false,
                         IsPropagated>;

template <bool IsPropagated>
using allocator_copy_construction_policy_specified =
    PolicyAwareAllocator<tree_type::value_type, true, IsPropagated, false,
                         false, true>;

template <bool IsAlwaysEqual, bool IsPropagated>
using allocator_copy_policy_specified =
    PolicyAwareAllocator<tree_type::value_type, IsAlwaysEqual, false,
                         IsPropagated, false, true>;

template <bool IsAlwaysEqual, bool IsPropagated>
using allocator_move_policy_specified =
    PolicyAwareAllocator<tree_type::value_type, IsAlwaysEqual, false, false,
                         IsPropagated, true>;

template <bool IsPropagated>
using swap_tree_type = RBtree<int, int, std::less<int>,
                              allocator_swap_policy_specified<IsPropagated>>;

template <bool IsPropagated>
using copy_construction_tree_type =
    RBtree<int, int, std::less<int>,
           allocator_copy_construction_policy_specified<IsPropagated>>;

template <bool IsAlwaysEqual, bool IsPropagated>
using copy_tree_type =
    RBtree<int, int, std::less<int>,
           allocator_copy_policy_specified<IsAlwaysEqual, IsPropagated>>;

template <bool IsAlwaysEqual, bool IsPropagated>
using move_tree_type =
    RBtree<int, int, std::less<int>,
           allocator_move_policy_specified<IsAlwaysEqual, IsPropagated>>;

TEST(RBtreeAllocatorTest, SwapPropagatedFalse) {
  auto allocator_1 = allocator_swap_policy_specified<false>(1);
  auto allocator_2 = allocator_swap_policy_specified<false>(2);

  swap_tree_type<false> tree_1(allocator_1);
  swap_tree_type<false> tree_2(allocator_2);

  std::swap(tree_1, tree_2);

  ASSERT_EQ(tree_1.get_allocator().id, 1);
  ASSERT_EQ(tree_2.get_allocator().id, 2);
}

TEST(RBtreeAllocatorTest, SwapPropagatedTrue) {
  auto allocator_1 = allocator_swap_policy_specified<true>(1);
  auto allocator_2 = allocator_swap_policy_specified<true>(2);

  swap_tree_type<true> tree_1(allocator_1);
  swap_tree_type<true> tree_2(allocator_2);

  std::swap(tree_1, tree_2);

  ASSERT_EQ(tree_1.get_allocator().id, 2);
  ASSERT_EQ(tree_2.get_allocator().id, 1);
}

TEST(RBtreeAllocatorTest, SelectOnCopyConstructionFalse) {
  auto allocator = allocator_copy_construction_policy_specified<false>(1);

  copy_construction_tree_type<false> tree_1(allocator);
  copy_construction_tree_type<false> tree_2(tree_1);

  ASSERT_EQ(tree_1.get_allocator().id, 1);
  ASSERT_EQ(tree_2.get_allocator().id, decltype(allocator)::kDefaultId);
}

TEST(RBtreeAllocatorTest, SelectOnCopyConstructionTrue) {
  auto allocator = allocator_copy_construction_policy_specified<true>(1);

  copy_construction_tree_type<true> tree_1(allocator);
  copy_construction_tree_type<true> tree_2(tree_1);

  ASSERT_EQ(tree_1.get_allocator().id, 1);
  ASSERT_EQ(tree_2.get_allocator().id, 1);
}

TEST(RBtreeAllocatorTest, PropagateOnCopyAssignmentTrue) {
  auto allocator = allocator_copy_policy_specified<true, true>(1);

  copy_tree_type<true, true> tree_1(allocator);
  copy_tree_type<true, true> tree_2;
  tree_2 = tree_1;

  ASSERT_EQ(tree_1.get_allocator().id, 1);
  ASSERT_EQ(tree_2.get_allocator().id, 1);
}

#include <map>

TEST(RBtreeAllocatorTest, IsAlwaysEqualTrueAndPropagateOnCopyAssignmentFalse) {
  auto allocator_1 = allocator_copy_policy_specified<true, false>(1);
  auto allocator_2 = allocator_copy_policy_specified<true, false>(2);

  std::map<int, int, std::less<int>, decltype(allocator_1)> m_1(allocator_1);
  std::map<int, int, std::less<int>, decltype(allocator_1)> m_2(allocator_2);
  m_2 = m_1;
  //copy_tree_type<true, false> tree_1(allocator_1);
  //copy_tree_type<true, false> tree_2(allocator_2);
  //tree_2 = tree_1;

  //ASSERT_EQ(tree_1.get_allocator().id, 1);
  //ASSERT_EQ(tree_2.get_allocator().id, 2);

  ASSERT_EQ(m_1.get_allocator().id, 1);
  ASSERT_EQ(m_2.get_allocator().id, 2);
}

TEST(RBtreeAllocatorTest, IsAlwaysEqualFalseAndPropagateOnCopyAssignmentFalseAndAllocatorsEqual) {
  auto allocator_1 = allocator_copy_policy_specified<false, false>(1);
  auto allocator_2 = allocator_copy_policy_specified<false, false>(1);

  copy_tree_type<false, false> tree_1(allocator_1);
  copy_tree_type<false, false> tree_2(allocator_2);
  tree_2 = tree_1;

  ASSERT_EQ(tree_1.get_allocator().id, 1);
  ASSERT_EQ(tree_2.get_allocator().id, 1);
}

TEST(RBtreeAllocatorTest, IsAlwaysEqualFalseAndPropagateOnCopyAssignmentFalseAndAllocatorsNotEqual) {
  auto allocator_1 = allocator_copy_policy_specified<false, false>(1);
  auto allocator_2 = allocator_copy_policy_specified<false, false>(2);

  copy_tree_type<false, false> tree_1(allocator_1);
  copy_tree_type<false, false> tree_2(allocator_2);
  tree_2 = tree_1;

  ASSERT_EQ(tree_1.get_allocator().id, 1);
  ASSERT_EQ(tree_2.get_allocator().id, 1);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}