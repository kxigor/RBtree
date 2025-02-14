#include <algorithm>
#include <iostream>
#include <numeric>
#include <random>

#include "src/RBtree.hpp"

int main() {
  RBtree<int, int> mtree;
  int arr[100] {};
  std::iota(std::begin(arr), std::end(arr), 0);
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(std::begin(arr), std::end(arr), g);


  for(int i = 0; i < 100; ++i) {
    mtree.insert({arr[i], arr[i]});
  }

  auto it = mtree.begin();

  std::cout << std::prev(mtree.end())->first << ' ' << std::endl;
}