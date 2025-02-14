#include <algorithm>
#include <iostream>
#include <map>
#include <numeric>
#include <random>

#include "src/RBtree.hpp"

int main() {
  constexpr const std::size_t kArrSize = 100;

  RBtree<int, int> mtree;

  int arr[kArrSize]{};
  std::iota(std::begin(arr), std::end(arr), 0);
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(std::begin(arr), std::end(arr), g);

  for (int i = 0; i < static_cast<int>(kArrSize); ++i) {
    mtree.insert({arr[i], arr[i]});
  }

  std::cout << std::prev(mtree.end())->first << ' '
            << std::prev(mtree.end())->first << std::endl;
}