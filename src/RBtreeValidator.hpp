#pragma once
#include "RBtreeFriendMediator.hpp"

template <class Key, class T, class Compare, class Allocator>
class RBtreeValidator
    : public RBtreeFriendMediator<Key, T, Compare, Allocator> {
 public:
  using mediator_type = RBtreeFriendMediator<Key, T, Compare, Allocator>;
  using tree_type = mediator_type::tree_type;
  using node_type = mediator_type::node_type;
  using mediator_type::RBtreeFriendMediator;
};

template <class Key, class T, class Compare = std::less<Key>,
          class Allocator = std::allocator<std::pair<const Key, T>>>
RBtreeValidator(RBtree<Key, T, Compare, Allocator>&)
    -> RBtreeValidator<Key, T, Compare, Allocator>;