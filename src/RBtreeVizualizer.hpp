#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <memory>
#include <cstdint>

template <class Key, class T, class Compare, class Allocator>
class RBtree<Key, T, Compare, Allocator>::RBtreeVisualizer {
 public:
  static void GenGraphRB(const RBtree<Key, T, Compare, Allocator>& tree) {
    std::ofstream file("graph.dot");
    assert(file.is_open());

    // Настройка графа
    file << "digraph G {\n";
    file << "  rankdir=TB;\n";
    file << "  nodesep=0.5;\n";
    file << "  ranksep=0.5;\n";
    file << "  node [shape=circle, style=filled, fixedsize=true, width=1.0];\n";

    GenGraphRecRB(tree, tree.root_, file);

    file << "}\n";

    file.close();
    system("dot -Tpng graph.dot -o graph.png");
  }

 private:
  static void GenGraphRecRB(
      const RBtree<Key, T, Compare, Allocator>& tree,
      typename RBtree<Key, T, Compare, Allocator>::BasicNode* node,
      std::ofstream& file) {
    if (tree.is_nil(node)) {
      return;
    }

    uintptr_t node_ptr = reinterpret_cast<uintptr_t>(node);
    uintptr_t parent_ptr = reinterpret_cast<uintptr_t>(node->parent);
    uintptr_t left_ptr = reinterpret_cast<uintptr_t>(node->left);
    uintptr_t right_ptr = reinterpret_cast<uintptr_t>(node->right);

    std::string node_color = node->is_red() ? "red" : "black";
    std::string font_color = node->is_red() ? "white" : "white";

    file << "  node" << node_ptr << " [label=\"" << node->get_key() 
         << "\", fillcolor=" << node_color 
         << ", fontcolor=" << font_color << "];\n";

    if (!tree.is_nil(node->parent)) {
      file << "  node" << node_ptr << " -> node" << parent_ptr 
           << " [color=blue];\n";
    }

    if (!tree.is_nil(node->left)) {
      file << "  node" << node_ptr << " -> node" << left_ptr 
           << " [color=green];\n";
      GenGraphRecRB(tree, node->left, file);
    }

    if (!tree.is_nil(node->right)) {
      file << "  node" << node_ptr << " -> node" << right_ptr 
           << " [color=red];\n";
      GenGraphRecRB(tree, node->right, file);
    }
  }
};