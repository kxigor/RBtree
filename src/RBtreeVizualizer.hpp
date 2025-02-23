#pragma once
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "RBtreeFriendMediator.hpp"

template <class Key, class T, class Compare, class Allocator>
class RBtreeVisualizer
    : public RBtreeFriendMediator<Key, T, Compare, Allocator> {
  static constexpr const char* kTitleText = "Red-Black tree by KXI";
  static constexpr const char* kTitleBgColor = "white";
  static constexpr const char* kTitleFontColor = "black";
  static constexpr int kTitleFontSize = 20;

  static constexpr double kNodeSeparation = 0.5;
  static constexpr double kRankSeparation = 0.5;

  static constexpr const char* kRedNodeColor = "red";
  static constexpr const char* kBlackNodeColor = "black";
  static constexpr const char* kNodeFontColor = "white";
  static constexpr const char* kLeftEdgeColor = "green";
  static constexpr const char* kRightEdgeColor = "red";
  static constexpr const char* kParentEdgeColor = "gray";
  static constexpr const char* kParentEdgeStyle = "dashed";

  static constexpr const char* kDotFilename = "graph.dot";
  static constexpr const char* kPngFilename = "graph.png";

  static constexpr const char* kGenerateErrorMessage =
      "Failed to generate PNG from DOT file.";

 public:
  using mediator_type = RBtreeFriendMediator<Key, T, Compare, Allocator>;
  using tree_type = typename mediator_type::tree_type;
  using node_type = typename mediator_type::node_type;

  using mediator_type::RBtreeFriendMediator;

  RBtreeVisualizer(tree_type& tree) : mediator_type(tree) {
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  }

  void Visualize() {
    file.open(kDotFilename);

    file << "digraph G {\n";
    file << "  rankdir=TB;\n";
    file << "  nodesep=" << kNodeSeparation << ";\n";
    file << "  ranksep=" << kRankSeparation << ";\n";
    file << "  node [shape=circle, style=filled, fontname=\"Arial\", "
            "fontsize=14];\n";

    file << "  labelloc=\"t\";\n";
    file << "  label=<<table border=\"1\" cellborder=\"0\" cellspacing=\"0\" "
            "cellpadding=\"4\">\n";
    file << "    <tr><td border=\"1\" bgcolor=\"" << kTitleBgColor << "\">\n";
    file << "      <font color=\"" << kTitleFontColor << "\" point-size=\""
         << kTitleFontSize << "\"><b>" << kTitleText << "</b></font>\n";
    file << "    </td></tr>\n";
    file << "  </table>>;\n";

    VisualizeRecursive(this->get_root());

    file << "}\n";

    file.close();
    ExecuteGenerateCommand();
  }

 private:
  void VisualizeRecursive(const node_type* node) {
    if (node->is_nil()) {
      return;
    }

    uintptr_t node_ptr = reinterpret_cast<uintptr_t>(node);
    uintptr_t parent_ptr = reinterpret_cast<uintptr_t>(node->parent);
    uintptr_t left_ptr = reinterpret_cast<uintptr_t>(node->left);
    uintptr_t right_ptr = reinterpret_cast<uintptr_t>(node->right);

    std::string node_color = node->is_red() ? kRedNodeColor : kBlackNodeColor;

    file << "  node" << node_ptr << " [label=\"key: " << node->get_key()
         << "\nmapped: " << node->get_mapped() << "\naddr: " << std::hex
         << node_ptr << std::dec << "\", fillcolor=" << node_color
         << ", fontcolor=" << kNodeFontColor << "];\n";

    if (node->left->is_not_nil()) {
      file << "  node" << node_ptr << " -> node" << left_ptr
           << " [color=" << kLeftEdgeColor
           << ", label=\"left\", labelfloat=true];\n";
      VisualizeRecursive(node->left);
    }

    if (node->right->is_not_nil()) {
      file << "  node" << node_ptr << " -> node" << right_ptr
           << " [color=" << kRightEdgeColor
           << ", label=\"right\", labelfloat=true];\n";
      VisualizeRecursive(node->right);
    }

    if (node->parent->is_not_nil()) {
      file << "  node" << node_ptr << " -> node" << parent_ptr
           << " [color=" << kParentEdgeColor << ", style=" << kParentEdgeStyle
           << "];\n";
    }
  }

  void ExecuteGenerateCommand() {
    std::string command;
    command += "dot -Tpng ";
    command += kDotFilename;
    command += " -o ";
    command += kPngFilename;
    if (std::system(command.c_str()) != 0) {
      throw std::runtime_error(kGenerateErrorMessage);
    }
  }

  std::ofstream file;
};

template <class Key, class T, class Compare = std::less<Key>,
          class Allocator = std::allocator<std::pair<const Key, T>>>
RBtreeVisualizer(RBtree<Key, T, Compare, Allocator>&)
    -> RBtreeVisualizer<Key, T, Compare, Allocator>;