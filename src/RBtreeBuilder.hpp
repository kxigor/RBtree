#pragma once
#include <functional>
#include <memory>

#include "RBtree.hpp"
#include "RBtreeFriendMediator.hpp"

template <class Key, class T, class Compare, class Allocator>
class RBtreeBuilder : public RBtreeFriendMediator<Key, T, Compare, Allocator> {
 public:
  using mediator_type = RBtreeFriendMediator<Key, T, Compare, Allocator>;
  using tree_type = typename mediator_type::tree_type;
  using node_type = typename mediator_type::node_type;
  using valued_node_type = typename mediator_type::valued_node_type;
  using color_type = node_type::Color;

  using mediator_type::RBtreeFriendMediator;

  enum class direction_type : bool {
    Right = false,
    Left = true,
  };

  RBtreeBuilder& add_node(Key key, color_type color, Key parent_key,
                          direction_type direction) {
    (void)key;
    (void)color;
    (void)parent_key;
    (void)direction;
    auto parent_it = this->get_tree().find(parent_key);
    assert(parent_it != this->get_tree().end());
    auto* parent_node = this->iterator_to_node_pointer(parent_it);
    
    valued_node_type* new_node = this->create_new_node(key, T());
    if(direction == direction_type::Left) {
      assert(parent_node->left == this->get_nil());
      parent_node->left = new_node;
      new_node->parent = parent_node;
    } else {
      assert(parent_node->right == this->get_nil());
      parent_node->right = new_node;
      new_node->parent = parent_node;
    }

    new_node->color = color;

    return *this;
  }

  RBtreeBuilder& root(Key key, color_type color) {
    valued_node_type* new_node = this->create_new_node(key, T());
    new_node->color = color;
    this->set_root(new_node);
    return *this;
  }

  tree_type& finalize() { 
    this->get_nil()->left = this->get_root();
    this->get_nil()->right = this->get_root()->get_most_left();
    return this->get_tree();
   }
};

template <class Key, class T, class Compare = std::less<Key>,
          class Allocator = std::allocator<std::pair<const Key, T>>>
RBtreeBuilder(RBtree<Key, T, Compare, Allocator>&)
    -> RBtreeBuilder<Key, T, Compare, Allocator>;