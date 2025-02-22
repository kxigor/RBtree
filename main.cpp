#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <numeric>
#include <random>

#include "src/RBtree.hpp"
#include "src/RBtreeTestConstructor.hpp"
#include "src/RBtreeValidator.hpp"
#include "src/RBtreeVizualizer.hpp"

int main() {
  RBtree<int, int> tree;
  RBtreeValidator validator(tree);
  std::cout << validator.validate() << std::endl;
  tree.emplace(1, 1);
  tree.emplace(2, 2);
  tree.emplace(3, 3);
  RBtreeVisualizer viz(tree);
  viz.GenGraphRB();
  std::cout << validator.validate() << std::endl;
  std::cout << validator << std::endl;
}