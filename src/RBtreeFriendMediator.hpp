#pragma once
#include "RBtree.hpp"

template <class Key, class T, class Compare, class Allocator>
class RBtreeFriendMediator {
 public:
  using tree_type = RBtree<Key, T, Compare, Allocator>;
  using node_type = typename tree_type::basic_node_type;

  RBtreeFriendMediator() = delete;
  RBtreeFriendMediator(tree_type& tree) : tree_(tree) {}

 protected:
  auto& get_root() { return tree_.root_; }
  auto& get_root() const { return tree_.root_; }
  auto& get_NIL() { return tree_.NIL_; }
  auto& get_NIL() const { return tree_.NIL_; }
  auto& get_compare() { return tree_.compare_; }
  auto& get_compare() const { return tree_.compare_; }
  auto& get_tree() { return tree_; }
  auto& get_tree() const { return tree_; }

 private:
  tree_type& tree_;
};