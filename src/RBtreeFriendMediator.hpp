#pragma once
#include "RBtree.hpp"

template <class Key, class T, class Compare, class Allocator>
class RBtreeFriendMediator {
 public:
  using tree_type = RBtree<Key, T, Compare, Allocator>;
  using node_type = typename tree_type::node_type;
  using valued_node_type = typename tree_type::valued_node_type;

  explicit RBtreeFriendMediator(tree_type& tree) : tree_(tree) {}

  RBtreeFriendMediator() = delete;

  void set_root(node_type* new_root) { tree_.root_ = new_root; }

  node_type* get_root() { return tree_.root_; }
  const node_type* get_root() const { return tree_.root_; }

  node_type* get_nil() { return tree_.NIL_; }
  const node_type* get_nil() const { return tree_.NIL_; }

  Compare& get_compare() { return tree_.compare_; }
  const Compare& get_compare() const { return tree_.compare_; }

  tree_type& get_tree() { return tree_; }
  const tree_type& get_tree() const { return tree_; }

  node_type* iterator_to_node_pointer(tree_type::iterator it) {
    return it.current_node_;
  }

  template <typename... Args>
  valued_node_type* create_new_node(Args&&... args) {
    valued_node_type* new_node = tree_.allocate();
    tree_.construct(new_node, get_nil(), get_nil(), get_nil(),
                    node_type::Color::Red, false,
                    std::forward<Args>(args)...);
    return new_node;
  }

 private:
  tree_type& tree_;
};