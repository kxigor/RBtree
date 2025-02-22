#pragma once
#include "RBtree.hpp"

template <class Key, class T, class Compare, class Allocator>
class RBtreeFriendMediator {
 public:
  using tree_type = RBtree<Key, T, Compare, Allocator>;
  using node_type = tree_type::basic_node_type;

  RBtreeFriendMediator() = delete;
  RBtreeFriendMediator(tree_type& tree) : tree_(tree) {}

  auto& get_root() { return tree_.root_; }
  auto& get_NIL() { return tree_.NIL_; }
  auto& get_compare() { return tree_.compare_; }

 private:
  tree_type& tree_;
};