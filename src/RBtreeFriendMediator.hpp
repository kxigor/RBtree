#pragma once
#include "RBtree.hpp"

template <class Key, class T, class Compare, class Allocator>
class RBtreeFriendMediator {
 public:
  using tree_type = RBtree<Key, T, Compare, Allocator>;
  using node_type = typename tree_type::node_type;
  using valued_node_type = typename tree_type::valued_node_type;
  using color_type = typename tree_type::color_type;

  explicit RBtreeFriendMediator(tree_type& tree) : tree_(tree) {}

  RBtreeFriendMediator() = delete;

  void set_root(node_type* new_root) noexcept { tree_.root_ = new_root; }

  [[nodiscard]] node_type* get_root() noexcept { return tree_.root_; }
  [[nodiscard]] const node_type* get_root() const noexcept {
    return tree_.root_;
  }

  [[nodiscard]] node_type* get_nil() noexcept { return tree_.NIL_; }
  [[nodiscard]] const node_type* get_nil() const noexcept { return tree_.NIL_; }

  [[nodiscard]] Compare& get_compare() noexcept { return tree_.compare_; }
  [[nodiscard]] const Compare& get_compare() const noexcept {
    return tree_.compare_;
  }

  [[nodiscard]] tree_type& get_tree() noexcept { return tree_; }
  [[nodiscard]] const tree_type& get_tree() const noexcept { return tree_; }

  [[nodiscard]] node_type* iterator_to_node_pointer(
      typename tree_type::iterator iter) noexcept {
    return iter.current_node_;
  }

  [[nodiscard]] const node_type* iterator_to_node_pointer(
      typename tree_type::iterator iter) const noexcept {
    return iter.current_node_;
  }

  [[nodiscard]] node_type* iterator_to_node_pointer(
      typename tree_type::const_iterator iter) noexcept {
    return iter.current_node_;
  }

  [[nodiscard]] const node_type* iterator_to_node_pointer(
      typename tree_type::const_iterator iter) const noexcept {
    return iter.current_node_;
  }

  template <typename... Args>
  [[nodiscard]] valued_node_type* create_new_node(Args&&... args) {
    valued_node_type* new_node = tree_.allocate();
    tree_.construct(new_node, get_nil(), get_nil(), get_nil(), color_type::Red,
                    false, std::forward<Args>(args)...);
    return new_node;
  }

  void increase_size(std::size_t offset) noexcept {
    tree_.increase_size(offset);
  }

  void decrease_size(std::size_t offset) noexcept {
    tree_.decrease_size(offset);
  }

  void normalizate_nil() noexcept { tree_.normalizate_nil(); }

 private:
  tree_type& tree_;
};