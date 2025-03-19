#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <numeric>
#include <random>

#include "src/RBtree.hpp"
#include "src/RBtreeBuilder.hpp"
#include "src/RBtreeValidator.hpp"
#include "src/RBtreeVizualizer.hpp"
using color_type =
    RBtreeBuilder<int, int, std::less<int>,
                  std::allocator<std::pair<const int, int>>>::color_type;

using direction_type =
    RBtreeBuilder<int, int, std::less<int>,
                  std::allocator<std::pair<const int, int>>>::direction_type;
int main() {
  RBtree<int, int> tree;
  RBtreeBuilder builder(tree);

  builder.root(50, color_type::Black)
      .add_node(30, color_type::Red, 50, direction_type::Left)
      .add_node(20, color_type::Red, 30, direction_type::Left)
      .finalize();

  
  RBtreeVisualizer visualizer(tree);
  visualizer.Visualize();

  tree.emplace(10, 0);

  visualizer.Visualize();
}