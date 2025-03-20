#pragma once
#include <bitset>
#include <iomanip>
#include <queue>
#include <ranges>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include "RBtreeFriendMediator.hpp"

template <class Key, class T, class Compare, class Allocator>
class RBtreeValidator
    : public RBtreeFriendMediator<Key, T, Compare, Allocator> {
 public:
  enum ErrorFlags {
    InvalidParents,
    CyclesDetected,
    InvalidHeight,
    InvalidColoring,
    NilNodeError,
    InvalidBST,
    InvalidSize,
    /*Service enum*/
    Count
  };

  static constexpr std::array<std::string_view, Count> kErrorMessages = {
      "Invalid parents",  "Cycles detected", "Invalid height",
      "Invalid coloring", "Invalid Nil",     "Invalid bst properties",
      "Invalid size"};

  static constexpr size_t compute_max_message_length() {
    size_t max_length = 0;
    for (const auto& message : kErrorMessages) {
      max_length = std::max(max_length, message.length());
    }
    return max_length;
  }

  static constexpr size_t kMaxMessageLength = compute_max_message_length();

  using mediator_type = RBtreeFriendMediator<Key, T, Compare, Allocator>;
  using tree_type = typename mediator_type::tree_type;
  using node_type = typename mediator_type::node_type;
  using errors_type = typename std::bitset<Count>;

  using mediator_type::RBtreeFriendMediator;

  friend std::ostream& operator<<(std::ostream& out,
                                  const RBtreeValidator& validator) {
    validator.validate();
    out << std::setw(kMaxMessageLength) << std::left << "Number of errors"
        << ": " << validator.errors_.count() << "\n";
    for (size_t i = 0; i < Count; ++i) {
      out << std::setw(kMaxMessageLength) << std::left << kErrorMessages.at(i)
          << ": ";
      if (validator.errors_.test(i)) {
        out << "FAIL";
      } else {
        out << "OK";
      }
      out << "\n";
    }
    return out;
  }

  bool check_parents() const {
    for (const auto& node : get_bfs()) {
      if (node->left->is_not_nil()) {
        if (node != node->left->parent) {
          return false;
        }
      }
      if (node->right->is_not_nil()) {
        if (node != node->right->parent) {
          return false;
        }
      }
    }

    return true;
  }

  bool check_cycles() const {
    enum class NodeState : int8_t { NotVisited, InProcess, Visited };

    std::unordered_map<const node_type*, NodeState> node_state;
    std::vector<const node_type*> dfs;

    dfs.push_back(this->get_root());

    while (!dfs.empty()) {
      const node_type* node = dfs.back();
      dfs.pop_back();

      if (node->is_nil()) {
        continue;
      }

      if (node_state[node] == NodeState::InProcess) {
        node_state[node] = NodeState::Visited;
        continue;
      }

      node_state[node] = NodeState::InProcess;

      if (node_state[node->left] == NodeState::InProcess) {
        return false;
      }
      if (node_state[node->right] == NodeState::InProcess) {
        return false;
      }
      dfs.push_back(node->left);
      dfs.push_back(node->right);
    }

    return true;
  }

  bool check_redblack_height() const {
    std::unordered_map<const node_type*, std::size_t> redblack_height;

    for (const auto& node : get_reversed_bfs()) {
      auto left_height = redblack_height[node->left];
      auto right_height = redblack_height[node->right];
      if (left_height != right_height) {
        return false;
      }
      redblack_height[node] =
          left_height + static_cast<std::size_t>(node->is_black());
    }

    return true;
  }

  bool check_coloring() const {
    if (this->get_root()->is_red()) {
      return false;
    }

    for (const auto& node : get_bfs()) {
      if (node->is_red()) {
        if (node->left->is_red() || node->right->is_red()) {
          return false;
        }
      }
    }

    return true;
  }

  bool check_nil() const {
    const node_type* k_nil = this->get_nil();
    bool result = true;

    result &= k_nil->is_black();
    result &= k_nil->left == this->get_root();
    result &= k_nil->right ==
              this->iterator_to_node_pointer(this->get_tree().begin());
    result &= k_nil->right == this->get_root()->get_most_left();

    return result;
  }

  bool check_bst_properties() const {
    for (const auto& node : get_bfs()) {
      if (node->left->is_not_nil()) {
        if (this->get_compare()(node->get_key(), node->left->get_key())) {
          return false;
        }
      }
      if (node->right->is_not_nil()) {
        if (this->get_compare()(node->right->get_key(), node->get_key())) {
          return false;
        }
      }
    }

    return true;
  }

  bool check_size() const {
    return get_bfs().size() == this->get_tree().size();
  }

  bool validate() const {
    errors_.reset();

    errors_.set(InvalidParents, !check_parents());
    errors_.set(CyclesDetected, !check_cycles());
    errors_.set(InvalidHeight, !check_redblack_height());
    errors_.set(InvalidColoring, !check_coloring());
    errors_.set(NilNodeError, !check_nil());
    errors_.set(InvalidBST, !check_bst_properties());
    errors_.set(InvalidSize, !check_size());

    return errors_.none();
  }

  errors_type get_errors_log() const { return errors_; }

 private:
  std::vector<const node_type*> get_bfs() const {
    std::vector<const node_type*> result;
    std::queue<const node_type*> nodes_to_visit;
    nodes_to_visit.push(this->get_root());
    while (!nodes_to_visit.empty()) {
      const node_type* current = nodes_to_visit.front();
      nodes_to_visit.pop();
      if (current->is_nil()) {
        continue;
      }
      result.push_back(current);
      nodes_to_visit.push(current->left);
      nodes_to_visit.push(current->right);
    }
    return result;
  }

  std::vector<const node_type*> get_reversed_bfs() const {
    auto result = get_bfs();
    std::ranges::reverse(result);
    return result;
  }

  mutable errors_type errors_{};
};

template <class Key, class T, class Compare = std::less<Key>,
          class Allocator = std::allocator<std::pair<const Key, T>>>
RBtreeValidator(RBtree<Key, T, Compare, Allocator>&)
    -> RBtreeValidator<Key, T, Compare, Allocator>;