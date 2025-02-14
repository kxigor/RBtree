#include <gtest/gtest.h>

#include <algorithm>
#include <numeric>
#include <random>

#include "RBtree.hpp"

static const constexpr int kInsSize = 100;
static const constexpr int kArrSize = 100;

TEST(RBTREE, CREATE) { RBtree<int, int> m; }

TEST(RBTREE, INSERT_PERFORMED) {
  RBtree<int, int> m;

  for (int i = 0; i < kInsSize; ++i) {
    m.insert({i, i});
  }
  ASSERT_EQ(m.size(), kInsSize);
}

TEST(RBTREE, INSERT_SORTING) {
  RBtree<int, int> m;

  int arr[kArrSize]{};
  std::iota(std::begin(arr), std::end(arr), 0);
  std::random_device random_device;
  std::mt19937 mt19937(random_device());
  std::shuffle(std::begin(arr), std::end(arr), mt19937);

  for (int i = 0; i < kArrSize; ++i) {
    m.insert({arr[i], arr[i]});
  }

  for (int i = 0; i < kArrSize; ++i) {
    /*TODO*/
  }
}

TEST(RBTREE, CLEAR) {
  RBtree<int, int> m;
  for (int i = 0; i < kInsSize; ++i) {
    m.insert({i, i});
  }

  m.clear();
  ASSERT_EQ(m.size(), 0);
}

TEST(RBTREE, ITERATOR_BEGIN_END_EQ) {
  RBtree<int, int> m;
  ASSERT_EQ(m.begin(), m.end());
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
