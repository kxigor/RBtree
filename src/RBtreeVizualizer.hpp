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

  static constexpr const char* kGenerateErrorMessage =
      "Failed to generate PNG from DOT file.";

 public:
  using mediator_type = RBtreeFriendMediator<Key, T, Compare, Allocator>;
  using tree_type = typename mediator_type::tree_type;
  using node_type = typename mediator_type::node_type;

  using mediator_type::RBtreeFriendMediator;

  explicit RBtreeVisualizer(tree_type& tree) : mediator_type(tree) {
    file_.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  }

  void visualize(const std::string& dot_name, const std::string& png_name) {
    file_.open(dot_name);

    file_ << "digraph G {\n";
    file_ << "  rankdir=TB;\n";
    file_ << "  nodesep=" << kNodeSeparation << ";\n";
    file_ << "  ranksep=" << kRankSeparation << ";\n";
    file_ << "  node [shape=circle, style=filled, fontname=\"Arial\", "
             "fontsize=14];\n";

    file_ << "  labelloc=\"t\";\n";
    file_ << "  label=<<table border=\"1\" cellborder=\"0\" cellspacing=\"0\" "
             "cellpadding=\"4\">\n";
    file_ << "    <tr><td border=\"1\" bgcolor=\"" << kTitleBgColor << "\">\n";
    file_ << "      <font color=\"" << kTitleFontColor << "\" point-size=\""
          << kTitleFontSize << "\"><b>" << kTitleText << "</b></font>\n";
    file_ << "    </td></tr>\n";
    file_ << "  </table>>;\n";

    visualize_recursive(this->get_root());

    file_ << "}\n";

    file_.close();
    execute_generate_command(dot_name, png_name);
  }

 private:
  void visualize_recursive(const node_type* node) {
    if (node->is_nil()) {
      return;
    }

    auto node_ptr = reinterpret_cast<uintptr_t>(node);
    auto parent_ptr = reinterpret_cast<uintptr_t>(node->parent);
    auto left_ptr = reinterpret_cast<uintptr_t>(node->left);
    auto right_ptr = reinterpret_cast<uintptr_t>(node->right);

    std::string node_color = node->is_red() ? kRedNodeColor : kBlackNodeColor;

    file_ << "  node" << node_ptr << " [label=\"key: " << node->get_key()
          << "\nmapped: " << node->get_mapped() << "\naddr: " << std::hex
          << node_ptr << std::dec << "\", fillcolor=" << node_color
          << ", fontcolor=" << kNodeFontColor << "];\n";

    if (node->left->is_not_nil()) {
      file_ << "  node" << node_ptr << " -> node" << left_ptr
            << " [color=" << kLeftEdgeColor
            << ", label=\"left\", labelfloat=true];\n";
      visualize_recursive(node->left);
    }

    if (node->right->is_not_nil()) {
      file_ << "  node" << node_ptr << " -> node" << right_ptr
            << " [color=" << kRightEdgeColor
            << ", label=\"right\", labelfloat=true];\n";
      visualize_recursive(node->right);
    }

    if (node->parent->is_not_nil()) {
      file_ << "  node" << node_ptr << " -> node" << parent_ptr
            << " [color=" << kParentEdgeColor << ", style=" << kParentEdgeStyle
            << "];\n";
    }
  }

  void execute_generate_command(const std::string& dot_name,
                                const std::string& png_name) {
    std::string command;
    command += "dot -Tpng ";
    command += dot_name;
    command += " -o ";
    command += png_name;
    if (std::system(command.c_str()) != 0) {
      throw std::runtime_error(kGenerateErrorMessage);
    }
  }

  std::ofstream file_;
};

template <class Key, class T, class Compare = std::less<Key>,
          class Allocator = std::allocator<std::pair<const Key, T>>>
RBtreeVisualizer(RBtree<Key, T, Compare, Allocator>&)
    -> RBtreeVisualizer<Key, T, Compare, Allocator>;