#include <gtest/gtest.h>

#include <map>
#include <memory>

#include "AuxiliaryAllocators.hpp"
#include "RBtree.hpp"

// clang-format off
using basic_key_type = int;
using basic_mapped_type = int;
using basic_compare_type = std::less<basic_key_type>;
using basic_allocator_type = std::allocator<std::pair<const basic_key_type, basic_mapped_type>>;
using basic_value_type = std::pair<const basic_key_type, basic_mapped_type>;

template <
  typename Key, 
  typename T, 
  typename Compare,
  typename Allocator
>
using tree_type = RBtree<Key, T, Compare, Allocator>;

template <
  bool IsAlwaysEqual, 
  bool SelectOnCopyConstruction,
  bool PropagateOnCopyAssignment, 
  bool PropagateOnMoveAssignment,
  bool PropagateOnSwap>
using allocator_fixed_value =
  PolicyAwareAllocator<
    basic_value_type, 
    IsAlwaysEqual,
    SelectOnCopyConstruction, 
    PropagateOnCopyAssignment,
    PropagateOnMoveAssignment, 
    PropagateOnSwap
  >;

template <typename Allocator>
using fixed_allocator_tree_type =
  tree_type<
    basic_key_type, 
    basic_mapped_type, 
    basic_compare_type, 
    Allocator
  >;

static constexpr const auto kDefaultAllocId =
  PolicyAwareAllocator<
    void, 
    false, 
    false, 
    false, 
    false, 
    false
  >::kDefaultId;
// clang-format on

template <typename T, T... Values>
constexpr auto sequence_to_tuple(std::integer_sequence<T, Values...>) {
  return std::make_tuple(Values...);
}

template <typename AllocatorFactory, typename InitIdsSeq, typename ExpectIdsSeq,
          bool... AllocatorParams>
struct TestCase {
  using Allocator =
      typename AllocatorFactory::template type<AllocatorParams...>;
  using Operation = AllocatorFactory::operation_type;

  static constexpr auto inited_ids = sequence_to_tuple(InitIdsSeq{});
  static constexpr auto expected_ids = sequence_to_tuple(ExpectIdsSeq{});
};

// clang-format off
struct SwapPolicyFactory {
  template <bool IsPropagated>
  using type = allocator_fixed_value<true, false, false, false, IsPropagated>;
  using operation_type = decltype([](auto& lhs, auto& rhs) { std::swap(lhs, rhs); });
};

struct CopyConstructionPolicyFactory {
  template <bool IsPropagated>
  using type = allocator_fixed_value<true, IsPropagated, false, false, true>;
  using operation_type = decltype([](auto& tree) -> auto { 
    return tree; 
  });
};

struct CopyAssignmentPolicyFactory {
  template <bool IsAlwaysEqual, bool IsPropagated>
  using type = allocator_fixed_value<IsAlwaysEqual, false, IsPropagated, false, true>;
  using operation_type = decltype([](auto& lhs, auto& rhs) { rhs = lhs; });
};

struct MoveAssignmentPolicyFactory {
  template <bool IsAlwaysEqual, bool IsPropagated>
  using type = allocator_fixed_value<IsAlwaysEqual, false, false, IsPropagated, true>;
  using operation_type = decltype([](auto& tree) -> auto { 
    std::remove_reference_t<decltype(tree)> result;
    result = std::move(tree);
    return result; 
  });
};
// clang-format on

template <typename T>
class RBtreePairWithOperationAllocatorParamTest : public ::testing::Test {};

TYPED_TEST_SUITE_P(RBtreePairWithOperationAllocatorParamTest);
TYPED_TEST_P(RBtreePairWithOperationAllocatorParamTest,
             PairWithOperationTests) {
  const auto& [inited1, inited2] = TypeParam::inited_ids;
  const auto& [expected1, expected2] = TypeParam::expected_ids;

  auto alloc1 = typename TypeParam::Allocator(inited1);
  auto alloc2 = typename TypeParam::Allocator(inited2);

  fixed_allocator_tree_type<decltype(alloc1)> tree1(alloc1);
  fixed_allocator_tree_type<decltype(alloc2)> tree2(alloc2);

  typename TypeParam::Operation operation;
  operation(tree1, tree2);

  EXPECT_EQ(tree1.get_allocator().id, expected1);
  EXPECT_EQ(tree2.get_allocator().id, expected2);
}
REGISTER_TYPED_TEST_SUITE_P(RBtreePairWithOperationAllocatorParamTest,
                            PairWithOperationTests);

template <typename T>
class RBtreeSingleWithOperationAllocatorParamTest : public ::testing::Test {};

TYPED_TEST_SUITE_P(RBtreeSingleWithOperationAllocatorParamTest);
TYPED_TEST_P(RBtreeSingleWithOperationAllocatorParamTest,
             SingleWithOperationTests) {
  const auto& [inited] = TypeParam::inited_ids;
  const auto& [expected] = TypeParam::expected_ids;

  auto alloc = typename TypeParam::Allocator(inited);

  fixed_allocator_tree_type<decltype(alloc)> tree(alloc);

  typename TypeParam::Operation operation;

  EXPECT_EQ(operation(tree).get_allocator().id, expected);
}
REGISTER_TYPED_TEST_SUITE_P(RBtreeSingleWithOperationAllocatorParamTest,
                            SingleWithOperationTests);

// clang-format off
using CopyConstructionTestCases = ::testing::Types<
  TestCase<
    CopyConstructionPolicyFactory,
    std::integer_sequence<int, 1>,
    std::integer_sequence<int, 1>,
    true
  >,
  TestCase<
    CopyConstructionPolicyFactory,
    std::integer_sequence<int, 1>,
    std::integer_sequence<int, kDefaultAllocId>,
    false
  >
>;
INSTANTIATE_TYPED_TEST_SUITE_P(
  CopyConstructionTests, 
  RBtreeSingleWithOperationAllocatorParamTest,
  CopyConstructionTestCases
);

using SwapTestCases = ::testing::Types<
  TestCase<
    SwapPolicyFactory, 
    std::integer_sequence<int, 1, 2>,
    std::integer_sequence<int, 1, 2>, 
    false
  >,
  TestCase<
    SwapPolicyFactory, 
    std::integer_sequence<int, 1, 2>,
    std::integer_sequence<int, 2, 1>, 
    true
  >
>;
INSTANTIATE_TYPED_TEST_SUITE_P(
  SwapTests, 
  RBtreePairWithOperationAllocatorParamTest,
  SwapTestCases
);

using CopyTestCases = ::testing::Types<
  TestCase<
    CopyAssignmentPolicyFactory,
    std::integer_sequence<int, 1, 2>,
    std::integer_sequence<int, 1, 1>,
    false, true
  >,
  TestCase<
    CopyAssignmentPolicyFactory,
    std::integer_sequence<int, 1, 2>,
    std::integer_sequence<int, 1, 2>,
    true, false
  >,
  TestCase<
    CopyAssignmentPolicyFactory,
    std::integer_sequence<int, 1, 1>,
    std::integer_sequence<int, 1, 1>,
    false, false
  >,
  TestCase<
    CopyAssignmentPolicyFactory,
    std::integer_sequence<int, 1, 2>,
    std::integer_sequence<int, 1, 2>,
    false, false
  >
>;
INSTANTIATE_TYPED_TEST_SUITE_P(
  CopyAssignmentTests,
  RBtreePairWithOperationAllocatorParamTest,
  CopyTestCases
);

using MoveTestCases = ::testing::Types<
  TestCase<
    MoveAssignmentPolicyFactory,
    std::integer_sequence<int, 1>,
    std::integer_sequence<int, 1>,
    false, true
  >,
  TestCase<
    MoveAssignmentPolicyFactory,
    std::integer_sequence<int, 1>,
    std::integer_sequence<int, kDefaultAllocId>,
    true, false
  >,
  TestCase<
    MoveAssignmentPolicyFactory,
    std::integer_sequence<int, 1>,
    std::integer_sequence<int, kDefaultAllocId>,
    false, false
  >,
  TestCase<
    MoveAssignmentPolicyFactory,
    std::integer_sequence<int, 1>,
    std::integer_sequence<int, kDefaultAllocId>,
    false, false
  >
>;
INSTANTIATE_TYPED_TEST_SUITE_P(
  MoveAssignmentTests,
  RBtreeSingleWithOperationAllocatorParamTest,
  MoveTestCases
);
// clang-format on

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}