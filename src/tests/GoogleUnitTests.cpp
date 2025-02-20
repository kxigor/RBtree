#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <numeric>
#include <random>
#include <ranges>

#include "RBtree.hpp"

static const constexpr int kInsSize = 5000;
static const constexpr int kArrSize = 5000;
static const constexpr int kAttempsWithRandomShuffle = 50;

void insert_sequence(auto& tree, auto start, auto stop, auto step = 1) {
  for (auto i = start; i < stop; i += step) {
    tree.insert({i, i});
  }
}

void insert_shuffled_sequence(auto& tree, auto from, auto arrsize) {
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

TEST(RBTREE, CREATE) { RBtree<int, int> m; }

TEST(RBTREE, INSERT_PERFORMED) {
  RBtree<int, int> m;
  ASSERT_NO_THROW(insert_sequence(m, 0, kInsSize, 1));
  ASSERT_EQ(m.size(), kInsSize);
}

TEST(RBTREE, EMPTY) {
  RBtree<int, int> m;
  ASSERT_TRUE(m.empty());
  m.insert({{}, {}});
  ASSERT_FALSE(m.empty());
}

TEST(RBTREE, GETALLOCATOR) {
  RBtree<int, int> m;
  m.get_allocator();
}

TEST(RBTREE, ELEMENT_ACCESS) {
  RBtree<int, int> m;

  insert_sequence(m, 1, 32, 2);

  for (int i = 0; i < 32; ++i) {
    if (i & 1) {
      ASSERT_EQ(m.at(i), i);
    } else {
      ASSERT_THROW(m.at(i), std::out_of_range);
    }
  }

  insert_sequence(m, 0, 32, 2);

  for (int i = 0; i < 32; ++i) {
    ASSERT_EQ(m[i], i);
  }
}

TEST(RBTREE, INSERT_SORTING) {
  RBtree<int, int> m;
  insert_shuffled_sequence(m, 0, kArrSize);

  for (int i = 0; i < kArrSize; ++i) {
    ASSERT_EQ(m[i], i);
  }
}

TEST(RBTREE, ITERATOR_BEGIN_END_EQ) {
  RBtree<int, int> m;
  ASSERT_EQ(m.begin(), m.end());
}

TEST(RBTREE, ITERATOR_FOLLOW_FORWARD) {
  RBtree<int, int> m;
  insert_shuffled_sequence(m, 0, kArrSize);
  auto it = m.begin();
  for (int i = 0; i < kInsSize; ++i) {
    ASSERT_EQ(it->first, i);
    ASSERT_EQ(it->second, i);
    ++it;
  }
}

TEST(RBTREE, ITERATOR_FOLLOW_BACKWARD) {
  RBtree<int, int> m;
  insert_shuffled_sequence(m, 0, kArrSize);
  auto it = m.end();
  for (int i = kInsSize - 1; i > 0; --i) {
    --it;
    ASSERT_EQ(it->first, i);
    ASSERT_EQ(it->second, i);
  }
}

TEST(RBTREE, ITERATOR_FORWARD_BACKWARD) {
  RBtree<int, int> m;
  insert_shuffled_sequence(m, 0, kArrSize);

  auto it = m.begin();

  for (int i = 0; i < kInsSize / 2; ++i) {
    ASSERT_EQ(it->first, i);
    ASSERT_EQ(it->second, i);
    ++it;
  }

  for (int i = kInsSize / 2; i > 0; --i) {
    ASSERT_EQ(it->first, i);
    ASSERT_EQ(it->second, i);
    --it;
  }
}

TEST(RBTREE, ITERATOR_PREV_END) {
  RBtree<int, int> m;
  insert_shuffled_sequence(m, 0, kArrSize);
  ASSERT_EQ(std::prev(m.end())->first, kInsSize - 1);
  ASSERT_EQ(std::prev(m.end())->second, kInsSize - 1);
}

TEST(RBTREE, ITERATOR_FORWARD_RANGE_LOOP) {
  RBtree<int, int> m;
  insert_shuffled_sequence(m, 0, kArrSize);
  int i = 0;
  for (auto el : m) {
    ASSERT_EQ(el.first, i);
    ASSERT_EQ(el.second, i);
    ++i;
  }
  i = 0;
  for (auto el : const_cast<const decltype(m)&>(m)) {
    ASSERT_EQ(el.first, i);
    ASSERT_EQ(el.second, i);
    ++i;
  }
}

TEST(RBTREE, ITERATOR_BACKWARD_RANGE_LOOP) {
  RBtree<int, int> m;
  insert_shuffled_sequence(m, 0, kArrSize);
  int i = kArrSize;
  for (auto el : std::views::reverse(m)) {
    --i;
    ASSERT_EQ(el.first, i);
    ASSERT_EQ(el.second, i);
  }
  i = kArrSize;
  for (auto el : std::views::reverse(const_cast<const decltype(m)&>(m))) {
    --i;
    ASSERT_EQ(el.first, i);
    ASSERT_EQ(el.second, i);
  }
}

TEST(RBTREE, CLEAR) {
  RBtree<int, int> m;
  insert_shuffled_sequence(m, 0, kArrSize);
  m.clear();
  ASSERT_TRUE(m.empty());
}

TEST(RBTREE, COUNT) {
  RBtree<int, int> m;
  insert_shuffled_sequence(m, 0, kArrSize);

  for (int i = 0; i < kInsSize; ++i) {
    ASSERT_EQ(m.count(i), 1);
  }

  ASSERT_EQ(m.count(-1), 0);
  ASSERT_EQ(m.count(kInsSize), 0);
}

TEST(RBTREE, FIND) {
  RBtree<int, int> m;
  insert_shuffled_sequence(m, 0, kArrSize);

  for (int i = 0; i < kInsSize; ++i) {
    auto it = m.find(i);
    ASSERT_NE(it, m.end());
    ASSERT_EQ(it->second, i);
  }

  ASSERT_EQ(m.find(-1), m.end());
  ASSERT_EQ(m.find(kInsSize), m.end());
}

TEST(RBTREE, CONTAINS) {
  RBtree<int, int> m;
  insert_shuffled_sequence(m, 0, kArrSize);

  for (int i = 0; i < kInsSize; ++i) {
    ASSERT_TRUE(m.contains(i));
  }

  ASSERT_FALSE(m.contains(-1));
  ASSERT_FALSE(m.contains(kInsSize));
}

TEST(RBTREE, EQUAL_RANGE) {
  RBtree<int, int> m;
  insert_shuffled_sequence(m, 0, kArrSize);

  for (int i = 0; i < kInsSize - 1; ++i) {
    auto range = m.equal_range(i);
    ASSERT_EQ(range.first->first, i);
    ASSERT_EQ(range.second->first, i + 1);
  }

  auto range = m.equal_range(kInsSize);
  ASSERT_EQ(range.first, m.end());
  ASSERT_EQ(range.second, m.end());
}

TEST(RBTREE, LOWER_BOUND) {
  RBtree<int, int> m;
  insert_sequence(m, 0, kInsSize, 2);

  for (int i = 0; i < kInsSize - 1; ++i) {
    auto it = m.lower_bound(i);
    if (i % 2 == 0) {
      ASSERT_EQ(it->first, i);
    } else {
      ASSERT_EQ(it->first, i + 1);
    }
  }

  ASSERT_EQ(m.lower_bound(kInsSize - 1), m.end());
  ASSERT_EQ(m.lower_bound(kInsSize), m.end());
}

TEST(RBTREE, UPPER_BOUND) {
  RBtree<int, int> m;
  insert_sequence(m, 0, kInsSize, 2);

  for (int i = 0; i < kInsSize - 2; ++i) {
    auto it = m.upper_bound(i);
    if (i % 2 == 0) {
      ASSERT_EQ(it->first, i + 2);
    } else {
      ASSERT_EQ(it->first, i + 1);
    }
  }
  ASSERT_EQ(m.upper_bound(kInsSize - 2), m.end());
  ASSERT_EQ(m.upper_bound(kInsSize - 1), m.end());
  ASSERT_EQ(m.upper_bound(kInsSize), m.end());
}

TEST(RBTREE, EQUAL_OPERATOR) {
  RBtree<int, int> m1;
  RBtree<int, int> m2;

  ASSERT_TRUE(m1 == m2);

  insert_sequence(m1, 0, kInsSize, 1);
  insert_sequence(m2, 0, kInsSize, 1);
  ASSERT_TRUE(m1 == m2);

  m1.insert({kInsSize, kInsSize});
  ASSERT_FALSE(m1 == m2);
}

TEST(RBTREE, NOT_EQUAL_OPERATOR) {
  RBtree<int, int> m1;
  RBtree<int, int> m2;

  ASSERT_FALSE(m1 != m2);

  insert_sequence(m1, 0, kInsSize, 1);
  insert_sequence(m2, 0, kInsSize, 1);
  ASSERT_FALSE(m1 != m2);

  m1.insert({kInsSize, kInsSize});
  ASSERT_TRUE(m1 != m2);
}

TEST(RBTREE, LESS_OPERATOR) {
  RBtree<int, int> m1;
  RBtree<int, int> m2;

  insert_sequence(m1, 0, kInsSize, 1);
  ASSERT_TRUE(m2 < m1);
  ASSERT_FALSE(m1 < m2);

  RBtree<int, int> m3;
  insert_sequence(m3, 0, kInsSize / 2, 1);
  ASSERT_TRUE(m3 < m1);
  ASSERT_FALSE(m1 < m3);
}

TEST(RBTREE, LESS_OR_EQUAL_OPERATOR) {
  RBtree<int, int> m1;
  RBtree<int, int> m2;

  insert_sequence(m1, 0, kInsSize, 1);
  ASSERT_TRUE(m2 <= m1);
  ASSERT_FALSE(m1 <= m2);

  RBtree<int, int> m3;
  insert_sequence(m3, 0, kInsSize / 2, 1);
  ASSERT_TRUE(m3 <= m1);
  ASSERT_FALSE(m1 <= m3);

  RBtree<int, int> m4;
  insert_sequence(m4, 0, kInsSize, 1);
  ASSERT_TRUE(m1 <= m4);
  ASSERT_TRUE(m4 <= m1);
}

TEST(RBTREE, GREATER_OPERATOR) {
  RBtree<int, int> m1;
  RBtree<int, int> m2;

  insert_sequence(m1, 0, kInsSize, 1);
  ASSERT_TRUE(m1 > m2);
  ASSERT_FALSE(m2 > m1);

  RBtree<int, int> m3;
  insert_sequence(m3, kInsSize / 2, kInsSize, 1);
  ASSERT_TRUE(m3 > m1);
  ASSERT_FALSE(m1 > m3);
}

TEST(RBTREE, GREATER_OR_EQUAL_OPERATOR) {
  RBtree<int, int> m1;
  RBtree<int, int> m2;

  insert_sequence(m1, 0, kInsSize, 1);
  ASSERT_TRUE(m1 >= m2);
  ASSERT_FALSE(m2 >= m1);

  RBtree<int, int> m3;
  insert_sequence(m3, kInsSize / 2, kInsSize, 1);
  ASSERT_TRUE(m3 >= m1);
  ASSERT_FALSE(m1 >= m3);

  RBtree<int, int> m4;
  insert_sequence(m4, 0, kInsSize, 1);
  ASSERT_TRUE(m1 >= m4);
  ASSERT_TRUE(m4 >= m1);
}

TEST(RBTREE, SPACESHIP_OPERATOR) {
  RBtree<int, int> m1;
  RBtree<int, int> m2;

  insert_sequence(m1, 0, kInsSize, 1);
  ASSERT_TRUE((m2 <=> m1) < 0);
  ASSERT_TRUE((m1 <=> m2) > 0);

  RBtree<int, int> m3;
  insert_sequence(m3, 0, kInsSize / 2, 1);
  ASSERT_TRUE((m3 <=> m1) < 0);
  ASSERT_TRUE((m1 <=> m3) > 0);

  RBtree<int, int> m4;
  insert_sequence(m4, 0, kInsSize, 1);
  ASSERT_TRUE((m1 <=> m4) == 0);
  ASSERT_TRUE((m4 <=> m1) == 0);
}

TEST(RBTREE, ERASE) {
  RBtree<int, int> tree;
  insert_shuffled_sequence(tree, 0, kArrSize);
  for (int i = 0; i < kArrSize; ++i) {
    auto it = tree.find(i);
    ASSERT_EQ(it, tree.begin());
    ASSERT_NE(it, tree.end());
    tree.erase(it);
    ASSERT_FALSE(tree.contains(i));
    ASSERT_EQ(tree.size(), kArrSize - 1 - i);
  }
  ASSERT_TRUE(tree.empty());
}

TEST(RBTREE, ERASE_MIDDLE) {
  for (int attempts = 0; attempts < kAttempsWithRandomShuffle; ++attempts) {
    RBtree<int, int> tree;
    insert_shuffled_sequence(tree, 0, kArrSize);
    int ctr = 0;
    for (int i = kArrSize / 4 * 1; i < kArrSize / 4 * 3; ++i) {
      auto it = tree.find(i);
      ASSERT_NE(it, tree.end());
      tree.erase(it);
      ASSERT_FALSE(tree.contains(i));
      ++ctr;
      ASSERT_EQ(tree.size(), kArrSize - ctr);
    }
    ASSERT_EQ(tree.size(), kArrSize - ctr);
  }
}

TEST(RBTREE, ERASE_BY_KEY) {
  RBtree<int, int> tree;
  insert_shuffled_sequence(tree, 0, kArrSize);
  for (int i = 0; i < kArrSize; ++i) {
    ASSERT_TRUE(tree.contains(i));
    tree.erase(i);
    ASSERT_FALSE(tree.contains(i));
    ASSERT_EQ(tree.size(), kArrSize - 1 - i);
  }
  ASSERT_TRUE(tree.empty());
}

TEST(RBTREE, ERASE_RANGE) {
  const int kFirstBorder = kArrSize / 4 * 1;
  const int kLastBorder = kArrSize / 4 * 3;

  RBtree<int, int> tree;
  insert_shuffled_sequence(tree, 0, kArrSize);

  auto first = tree.find(kFirstBorder);
  auto last = tree.find(kLastBorder);

  tree.erase(first, last);

  for (int i = kFirstBorder; i < kLastBorder; ++i) {
    ASSERT_FALSE(tree.contains(i));
  }

  for (int i = 0; i < kFirstBorder; ++i) {
    ASSERT_TRUE(tree.contains(i));
  }
  for (int i = kLastBorder; i < kArrSize; ++i) {
    ASSERT_TRUE(tree.contains(i));
  }

  ASSERT_EQ(tree.size(), kArrSize - kLastBorder + kFirstBorder);
}

TEST(RBTREE, ERASE_IF) {
  const auto EvenPred = [](const RBtree<int, int>::value_type& val) noexcept {
    return !(val.first & 1);
  };
  const auto OddPred = [](const RBtree<int, int>::value_type& val) noexcept {
    return val.first & 1;
  };
  RBtree<int, int> m;
  insert_sequence(m, 0, kInsSize, 1);
  m.erase_if(EvenPred);
  ASSERT_EQ(m.size(), kInsSize / 2);
  for (const auto& [key, value] : m) {
    ASSERT_EQ(key % 2, 1);
  }
  m.erase_if(OddPred);
  ASSERT_TRUE(m.empty());
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
