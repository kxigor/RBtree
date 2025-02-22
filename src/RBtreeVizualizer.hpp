#pragma once
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "RBtreeFriendMediator.hpp"

template <class Key, class T, class Compare, class Allocator>
class RBtreeVisualizer
    : public RBtreeFriendMediator<Key, T, Compare, Allocator> {
 public:
  using mediator_type = RBtreeFriendMediator<Key, T, Compare, Allocator>;
  using tree_type = mediator_type::tree_type;
  using node_type = mediator_type::node_type;
  using mediator_type::RBtreeFriendMediator;

  void GenGraphRB() const {
    std::ofstream file("graph.dot");
    assert(file.is_open());

    file << "digraph G {\n";
    file << "  rankdir=TB;\n";
    file << "  nodesep=0.5;\n";
    file << "  ranksep=0.5;\n";
    file << "  node [shape=circle, style=filled];\n";

    file << "  labelloc=\"t\";\n";
    file << "  label=<<table border=\"1\" cellborder=\"0\" cellspacing=\"0\" "
            "cellpadding=\"4\">\n";
    file << "    <tr><td border=\"1\" bgcolor=\"black\">\n";
    file << "      <font color=\"red\" point-size=\"20\"><b>Red-Black tree "
            "by KXI</b></font>\n";
    file << "    </td></tr>\n";
    file << "  </table>>;\n";

    GenGraphRecRB(this->get_root(), file);

    file << "}\n";

    file.close();
    system("dot -Tpng graph.dot -o graph.png");
  }

 private:
  void GenGraphRecRB(const node_type* node, std::ofstream& file) const {
    if (node->is_nil()) {
      return;
    }

    uintptr_t node_ptr = reinterpret_cast<uintptr_t>(node);
    uintptr_t parent_ptr = reinterpret_cast<uintptr_t>(node->parent);
    uintptr_t left_ptr = reinterpret_cast<uintptr_t>(node->left);
    uintptr_t right_ptr = reinterpret_cast<uintptr_t>(node->right);

    std::string node_color = node->is_red() ? "red" : "black";
    std::string font_color = node->is_red() ? "white" : "white";

    file << "  node" << node_ptr << " [label=\"key:" << node->get_key()
         << "\nmapped:" << node->get_mapped() << "\naddr:" << std::hex
         << node_ptr << std::dec << "\", fillcolor=" << node_color
         << ", fontcolor=" << font_color << "];\n";

    if (node->left->is_not_nil()) {
      file << "  node" << node_ptr << " -> node" << left_ptr
           << " [color=green, label=\"left\", labelfloat=true];\n";
      GenGraphRecRB(node->left, file);
    }

    if (node->right->is_not_nil()) {
      file << "  node" << node_ptr << " -> node" << right_ptr
           << " [color=red, label=\"right\", labelfloat=true];\n";
      GenGraphRecRB(node->right, file);
    }

    if (node->parent->is_not_nil()) {
      file << "  node" << node_ptr << " -> node" << parent_ptr
           << " [color=blue];\n";
    }
  }
};

template <class Key, class T, class Compare = std::less<Key>,
          class Allocator = std::allocator<std::pair<const Key, T>>>
RBtreeVisualizer(RBtree<Key, T, Compare, Allocator>&)
    -> RBtreeVisualizer<Key, T, Compare, Allocator>;