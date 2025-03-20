#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <numeric>
#include <random>
#include <ranges>

#include "RBtree.hpp"
#include "RBtreeValidator.hpp"

static constexpr const int kInsertSize = 500000;
static constexpr const int kShuffledInsertSize = 5000;
static constexpr const int kShuffleAttempts = 50;
static constexpr const int kSortingInsertAttemps = 500;
static constexpr const int kLeftBorder = kShuffledInsertSize / 4 * 1;
static constexpr const int kRightBorder = kShuffledInsertSize / 4 * 3;

#define DO_ATTEMPTS(attemps_number, ...)                         \
  for (auto current_attemp = 0; current_attemp < attemps_number; \
       ++current_attemp) {                                       \
    __VA_ARGS__                                                  \
  }

void InsertSequence(auto& tree, auto start, auto stop, auto step = 1) {
  for (auto i = start; i < stop; i += step) {
    tree.insert({i, i});
  }
}

void InsertShuffledSequence(auto& tree, auto from, auto arrsize) {
  std::vector<int> vec(static_cast<std::size_t>(arrsize));
  std::iota(std::begin(vec), std::end(vec), from);

  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  auto millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

  std::mt19937 mt19937(static_cast<std::mt19937::result_type>(millis));

  std::shuffle(std::begin(vec), std::end(vec), mt19937);

  for (std::size_t i = 0; i < static_cast<std::size_t>(arrsize); ++i) {
    tree.insert({vec[i], vec[i]});
  }
}

RBtree<int, int> InitSequence(auto start, auto stop, auto step = 1) {
  RBtree<int, int> result;
  InsertSequence(result, start, stop, step);
  return result;
}

RBtree<int, int> InitShuffledSequence(auto from, auto arrsize) {
  RBtree<int, int> result;
  InsertShuffledSequence(result, from, arrsize);
  return result;
}

TEST(RBTREE, CREATE) { RBtree<int, int> tree; }

TEST(RBTREE, INSERT_PERFORMED) {
  RBtree<int, int> tree = InitSequence(0, kInsertSize, 1);
  ASSERT_EQ(tree.size(), kInsertSize);
}

TEST(RBTREE, EMPTY) {
  RBtree<int, int> tree;
  ASSERT_TRUE(tree.empty());
  tree.insert({{}, {}});
  ASSERT_FALSE(tree.empty());
}

TEST(RBTREE, GETALLOCATOR) {
  RBtree<int, int> tree;
  auto alloc = tree.get_allocator();
}

TEST(RBTREE, ELEMENT_ACCESS) {
  RBtree<int, int> tree = InitSequence(1, kInsertSize, 2);
  for (int i = 0; i < kInsertSize; ++i) {
    if (i & 1) {
      ASSERT_EQ(tree.at(i), i);
    } else {
      ASSERT_THROW(std::ignore = tree.at(i), std::out_of_range);
    }
  }
  InsertSequence(tree, 0, kInsertSize, 2);
  for (int i = 0; i < kInsertSize; ++i) {
    ASSERT_EQ(tree[i], i);
  }
}

TEST(RBTREE, INSERT_SORTING) {
  DO_ATTEMPTS(
      kSortingInsertAttemps,
      RBtree<int, int> tree = InitShuffledSequence(0, kShuffledInsertSize);
      for (int i = 0; i < kShuffledInsertSize; ++i) { ASSERT_EQ(tree[i], i); })
}

TEST(RBTREE, ITERATOR_BEGIN_END_EQ) {
  RBtree<int, int> tree;
  ASSERT_EQ(tree.begin(), tree.end());
}

TEST(RBTREE, ITERATOR_FOLLOW_FORWARD) {
  RBtree<int, int> tree;
  DO_ATTEMPTS(
      kShuffleAttempts, InsertShuffledSequence(tree, 0, kShuffledInsertSize);
      auto it = tree.begin(); for (int i = 0; i < kShuffledInsertSize; ++i) {
        ASSERT_EQ(it->first, i);
        ASSERT_EQ(it->second, i);
        ++it;
      } tree.clear();)
}

TEST(RBTREE, ITERATOR_FOLLOW_BACKWARD) {
  RBtree<int, int> tree;
  DO_ATTEMPTS(
      kShuffleAttempts, InsertShuffledSequence(tree, 0, kShuffledInsertSize);
      auto it = tree.end(); for (int i = kShuffledInsertSize - 1; i > 0; --i) {
        --it;
        ASSERT_EQ(it->first, i);
        ASSERT_EQ(it->second, i);
      } tree.clear();)
}

TEST(RBTREE, ITERATOR_FORWARD_BACKWARD) {
  RBtree<int, int> tree;
  DO_ATTEMPTS(
      kShuffleAttempts, InsertShuffledSequence(tree, 0, kShuffledInsertSize);
      auto it = tree.begin();
      for (int i = 0; i < kShuffledInsertSize / 2; ++i) {
        ASSERT_EQ(it->first, i);
        ASSERT_EQ(it->second, i);
        ++it;
      } for (int i = kShuffledInsertSize / 2; i > 0; --i) {
        ASSERT_EQ(it->first, i);
        ASSERT_EQ(it->second, i);
        --it;
      } tree.clear();)
}

TEST(RBTREE, ITERATOR_PREV_END) {
  RBtree<int, int> tree;
  DO_ATTEMPTS(kShuffleAttempts,
              InsertShuffledSequence(tree, 0, kShuffledInsertSize);
              ASSERT_EQ(std::prev(tree.end())->first, kShuffledInsertSize - 1);
              ASSERT_EQ(std::prev(tree.end())->second, kShuffledInsertSize - 1);
              tree.clear(););
}

TEST(RBTREE, ITERATOR_FORWARD_RANGE_LOOP) {
  RBtree<int, int> tree;
  DO_ATTEMPTS(
      kShuffleAttempts, InsertShuffledSequence(tree, 0, kShuffledInsertSize);
      int expected_key = 0; for (const auto& element : tree) {
        ASSERT_EQ(element.first, expected_key);
        ASSERT_EQ(element.second, expected_key);
        ++expected_key;
      } expected_key = 0;
      for (const auto& element : const_cast<const decltype(tree)&>(tree)) {
        ASSERT_EQ(element.first, expected_key);
        ASSERT_EQ(element.second, expected_key);
        ++expected_key;
      } tree.clear(););
}

TEST(RBTREE, ITERATOR_BACKWARD_RANGE_LOOP) {
  RBtree<int, int> tree;
  DO_ATTEMPTS(
      kShuffleAttempts, InsertShuffledSequence(tree, 0, kShuffledInsertSize);
      int expected_key = kShuffledInsertSize;
      for (const auto& element : std::views::reverse(tree)) {
        --expected_key;
        ASSERT_EQ(element.first, expected_key);
        ASSERT_EQ(element.second, expected_key);
      } expected_key = kShuffledInsertSize;
      for (const auto& element : std::views::reverse(
               const_cast<const decltype(tree)&>(tree))) {
        --expected_key;
        ASSERT_EQ(element.first, expected_key);
        ASSERT_EQ(element.second, expected_key);
      } tree.clear(););
}

TEST(RBTREE, CLEAR) {
  RBtree<int, int> tree;
  DO_ATTEMPTS(kShuffleAttempts,
              InsertShuffledSequence(tree, 0, kShuffledInsertSize);
              tree.clear(); ASSERT_TRUE(tree.empty()););
}

TEST(RBTREE, COUNT) {
  RBtree<int, int> tree = InitSequence(0, kInsertSize, 1);
  for (int i = 0; i < kInsertSize; ++i) {
    ASSERT_EQ(tree.count(i), 1);
  }
  ASSERT_EQ(tree.count(-1), 0);
  ASSERT_EQ(tree.count(kInsertSize), 0);
}

TEST(RBTREE, FIND) {
  RBtree<int, int> tree;
  DO_ATTEMPTS(
      kShuffleAttempts, InsertShuffledSequence(tree, 0, kShuffledInsertSize);
      for (int i = 0; i < kShuffledInsertSize; ++i) {
        auto it = tree.find(i);
        ASSERT_NE(it, tree.end());
        ASSERT_EQ(it->second, i);
      } ASSERT_EQ(tree.find(-1), tree.end());
      ASSERT_EQ(tree.find(kInsertSize), tree.end()); tree.clear();)
}

TEST(RBTREE, CONTAINS) {
  RBtree<int, int> tree = InitSequence(0, kInsertSize, 1);
  for (int i = 0; i < kInsertSize; ++i) {
    ASSERT_TRUE(tree.contains(i));
  }
  ASSERT_FALSE(tree.contains(-1));
  ASSERT_FALSE(tree.contains(kInsertSize));
}

TEST(RBTREE, LOWER_BOUND) {
  RBtree<int, int> tree = InitSequence(0, kInsertSize, 2);
  for (int i = 0; i < kInsertSize - 1; ++i) {
    auto it = tree.lower_bound(i);
    if (i % 2 == 0) {
      ASSERT_EQ(it->first, i);
    } else {
      ASSERT_EQ(it->first, i + 1);
    }
  }
  ASSERT_EQ(tree.lower_bound(kInsertSize - 1), tree.end());
  ASSERT_EQ(tree.lower_bound(kInsertSize), tree.end());
}

TEST(RBTREE, UPPER_BOUND) {
  RBtree<int, int> tree = InitSequence(0, kInsertSize, 2);
  for (int i = 0; i < kInsertSize - 2; ++i) {
    auto it = tree.upper_bound(i);
    if (i % 2 == 0) {
      ASSERT_EQ(it->first, i + 2);
    } else {
      ASSERT_EQ(it->first, i + 1);
    }
  }
  ASSERT_EQ(tree.upper_bound(kInsertSize - 2), tree.end());
  ASSERT_EQ(tree.upper_bound(kInsertSize - 1), tree.end());
  ASSERT_EQ(tree.upper_bound(kInsertSize), tree.end());
}

TEST(RBTREE, EQUAL_RANGE) {
  RBtree<int, int> tree = InitSequence(0, kInsertSize, 1);
  for (int i = 0; i < kInsertSize - 1; ++i) {
    auto range = tree.equal_range(i);
    ASSERT_EQ(range.first->first, i);
    ASSERT_EQ(range.second->first, i + 1);
  }
  auto range = tree.equal_range(kInsertSize);
  ASSERT_EQ(range.first, tree.end());
  ASSERT_EQ(range.second, tree.end());
}

TEST(RBTREE, SPACESHIP_OPERATOR) {
  RBtree<int, int> empty_tree1;
  RBtree<int, int> empty_tree2;
  ASSERT_TRUE((empty_tree1 <=> empty_tree2) == 0);

  RBtree<int, int> tree1;
  RBtree<int, int> tree2;
  InsertSequence(tree1, 0, kInsertSize, 1);
  InsertSequence(tree2, 0, kInsertSize, 1);
  ASSERT_TRUE((tree1 <=> tree2) == 0);

  RBtree<int, int> tree3;
  InsertSequence(tree3, 0, kInsertSize / 2, 1);
  ASSERT_TRUE((tree3 <=> tree1) < 0);
  ASSERT_TRUE((tree1 <=> tree3) > 0);

  RBtree<int, int> tree4;
  InsertSequence(tree4, kInsertSize / 2, kInsertSize, 1);
  ASSERT_TRUE((tree4 <=> tree1) > 0);
  ASSERT_TRUE((tree1 <=> tree4) < 0);

  RBtree<int, double> tree5;
  RBtree<int, double> tree6;
  for (int i = 0; i < kInsertSize; ++i) {
    tree5.insert({i, static_cast<double>(i)});
    tree6.insert({i, static_cast<double>(i)});
  }
  ASSERT_TRUE((tree5 <=> tree6) == 0);

  RBtree<int, int> tree7;
  RBtree<int, int> tree8;
  InsertSequence(tree7, 0, kInsertSize, 1);
  InsertSequence(tree8, 1, kInsertSize + 1, 1);
  ASSERT_TRUE((tree7 <=> tree8) < 0);
  ASSERT_TRUE((tree8 <=> tree7) > 0);

  RBtree<int, int> tree9;
  RBtree<int, int> tree10;
  InsertSequence(tree9, 0, kInsertSize, 1);
  InsertSequence(tree10, 0, kInsertSize, 1);
  tree10.insert({kInsertSize + 1, kInsertSize + 1});
  ASSERT_TRUE((tree9 <=> tree10) < 0);
  ASSERT_TRUE((tree10 <=> tree9) > 0);

  RBtree<int, int> tree11;
  RBtree<int, int> tree12;
  InsertSequence(tree11, 0, kInsertSize, 3);
  InsertSequence(tree12, 0, kInsertSize, 5);
  ASSERT_TRUE((tree11 <=> tree12) < 0);
  ASSERT_TRUE((tree12 <=> tree11) > 0);
}

TEST(RBTREE, ERASE) {
  RBtree<int, int> tree;
  DO_ATTEMPTS(
      kShuffleAttempts, InsertShuffledSequence(tree, 0, kShuffledInsertSize);
      for (int i = 0; i < kShuffledInsertSize; ++i) {
        auto it = tree.find(i);
        ASSERT_EQ(it, tree.begin());
        ASSERT_NE(it, tree.end());
        tree.erase(it);
        ASSERT_FALSE(tree.contains(i));
        ASSERT_EQ(tree.size(), kShuffledInsertSize - 1 - i);
      } ASSERT_TRUE(tree.empty());)
}

TEST(RBTREE, ERASE_MIDDLE) {
  RBtree<int, int> tree;
  DO_ATTEMPTS(
      kShuffleAttempts, InsertShuffledSequence(tree, 0, kShuffledInsertSize);
      int ctr = 0; auto begin = tree.find(0);
      for (int i = kLeftBorder; i < kRightBorder; ++i, ++ctr) {
        auto it = tree.find(i);
        ASSERT_EQ(tree.begin(), begin);
        ASSERT_NE(it, tree.end());
        tree.erase(it);
        ASSERT_FALSE(tree.contains(i));
        ASSERT_EQ(tree.size(), kShuffledInsertSize - ctr - 1);
      } ASSERT_EQ(tree.size(), kShuffledInsertSize - ctr);
      tree.clear();)
}

TEST(RBTREE, ERASE_BY_KEY) {
  RBtree<int, int> tree;
  DO_ATTEMPTS(
      kShuffleAttempts, InsertShuffledSequence(tree, 0, kShuffledInsertSize);
      for (int i = 0; i < kShuffledInsertSize; ++i) {
        ASSERT_TRUE(tree.contains(i));
        tree.erase(i);
        ASSERT_FALSE(tree.contains(i));
        ASSERT_EQ(tree.size(), kShuffledInsertSize - 1 - i);
      } ASSERT_TRUE(tree.empty());)
}

TEST(RBTREE, ERASE_RANGE) {
  RBtree<int, int> tree;
  DO_ATTEMPTS(
      kShuffleAttempts, InsertShuffledSequence(tree, 0, kShuffledInsertSize);
      auto first = tree.find(kLeftBorder); auto last = tree.find(kRightBorder);
      tree.erase(first, last);
      for (int i = kLeftBorder; i < kRightBorder; ++i) {
        ASSERT_FALSE(tree.contains(i));
      } for (int i = 0; i < kLeftBorder; ++i) {
        ASSERT_TRUE(tree.contains(i));
      } for (int i = kRightBorder; i < kShuffledInsertSize; ++i) {
        ASSERT_TRUE(tree.contains(i));
      } ASSERT_EQ(tree.size(),
                  kShuffledInsertSize - kRightBorder + kLeftBorder);
      tree.clear();)
}

TEST(RBTREE, ERASE_IF) {
  const auto EvenPred = [](const RBtree<int, int>::value_type& value) noexcept {
    return !(value.first & 1);
  };
  const auto OddPred = [](const RBtree<int, int>::value_type& value) noexcept {
    return value.first & 1;
  };
  RBtree<int, int> tree = InitSequence(0, kInsertSize, 1);
  tree.erase_if(EvenPred);
  ASSERT_EQ(tree.size(), kInsertSize / 2);
  for (const auto& value : tree) {
    ASSERT_TRUE(OddPred(value));
  }
  tree.erase_if(OddPred);
  ASSERT_TRUE(tree.empty());
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
