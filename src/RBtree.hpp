#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>

template <class Key, class T, class Compare = std::less<Key>,
          class Allocator = std::allocator<std::pair<const Key, T>>>
class RBtree {
 public:
  using key_type = Key;
  using mapped_type = T;
  using value_type = std::pair<const Key, T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using key_compare = Compare;
  using allocator_type = Allocator;
  using reference = value_type&;
  using const_reference = const value_type&;

  using allocator_traits = std::allocator_traits<allocator_type>;

  using pointer = typename allocator_traits::pointer;
  using const_pointer = typename allocator_traits::const_pointer;

  using iterator = void;       /*TODO*/
  using const_iterator = void; /*TODO*/
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  struct Node;

  struct BasicNode {
    enum class Color : bool { Red, Black };

    BasicNode* left;
    BasicNode* right;
    BasicNode* parent;

    Color color;

    bool is_left() const { return parent->left == this; }
    bool is_right() const { return parent->right == this; }

    bool is_red() const { return color == Color::Red; }
    bool is_black() const { return color == Color::Black; }

    const key_type& get_key() const {
      assert(dynamic_cast<const Node*>(this) != nullptr);
      return static_cast<const Node*>(this)->val.first;
    }

    mapped_type& get_mapped() {
      assert(dynamic_cast<Node*>(this) != nullptr);
      return static_cast<Node*>(this)->val.second;
    }

    const mapped_type& get_mapped() const {
      assert(dynamic_cast<const Node*>(this) != nullptr);
      return const_cast<BasicNode*>(this)->get_mapped();
    }
  };

  struct Node : BasicNode {
    value_type val;
  };

  using basic_node_type = BasicNode;
  using node_type = Node;
  using node_allocator_type = typename std::allocator_traits<
      allocator_type>::template rebind_alloc<node_type>;
  using node_allocator_traits = std::allocator_traits<node_allocator_type>;

  using Color = basic_node_type::Color;

  RBtree() { insert(&NIL_); }

  // template <class... Args>
  // std::pair<iterator, bool> emplace(Args&&... args) {

  // }

 private:
  void insert(basic_node_type* z) {
    basic_node_type* x = root_;
    basic_node_type* y = &NIL_;

    while (x != &NIL_) {
      y = x;
      if (compare_(x->get_key(), z->get_key())) {
        x = x->right;
      } else {
        x = x->left;
      }
    }

    z->parent = y;
    if (y == &NIL_) {
      root_ = z;
    } else {
      if (compare_(y->get_key(), z->get_key())) {
        y->right = z;
      } else {
        y->left = z;
      }
    }

    insert_fixup(z);
  }

  void insert_fixup(basic_node_type* current) {
    while (current->parent->is_red()) {
      if (current->parent->is_left()) {
        current = insert_handle_left_case(current);
      } else /*if (current->parent->is_right())*/ {
        current = insert_handle_right_case(current);
      }
    }
    root_->color = Color::Black;
  }

  basic_node_type* insert_handle_left_case(basic_node_type* current) {
    basic_node_type* parent = current->parent;
    basic_node_type* grandparent = parent->parent;
    basic_node_type* uncle = grandparent->right;
    if (uncle->is_red()) {
      parent->color = Color::Black;
      uncle->color = Color::Black;
      grandparent->color = Color::Red;
      current = grandparent;
    } else /*if (uncle->is_black())*/ {
      if (current->is_right()) {
        left_rotate(parent);
        std::swap(parent, current);
      }
      parent->color = Color::Black;
      grandparent->color = Color::Red;
      right_rotate(grandparent);
    }
  }

  basic_node_type* insert_handle_right_case(basic_node_type* current) {
    basic_node_type* parent = current->parent;
    basic_node_type* grandparent = parent->parent;
    basic_node_type* uncle = grandparent->left;
    if (uncle->is_red()) {
      parent->color = Color::Black;
      uncle->color = Color::Black;
      grandparent->color = Color::Red;
      current = grandparent;
    } else /*if (uncle->is_black())*/ {
        }
  }

  void left_rotate(basic_node_type* x) {
    basic_node_type* y = x->right;

    y->parent = x->parent;
    x->parent = y;

    if (y->parent->left == x) {
      y->parent->left = y;
    } else {
      y->parent->right = y;
    }

    if (y->parent == NIL_) {
      root_ = y;
    }

    x->right = y->left;
    x->right->parent = x;
    y->left = x;
  }

  void right_rotate(basic_node_type* y) {
    basic_node_type* x = y->left;

    x->parent = y->parent;
    y->parent = x;

    if (x->parent->left == y) {
      x->parent->left = x;
    } else {
      x->parent->right = x;
    }

    if (x->parent == &NIL_) {
      root_ = x;
    }

    y->left = x->right;
    y->left->parent = y;
    x->right = y;
  }

  node_type* allocate() { return node_allocator_traits::allocate(alloc_, 1); }

  template <typename... Args>
  void construct(node_type* object, Args&&... args) {
    node_allocator_traits::construct(alloc_, object,
                                     std::forward<Args>(args)...);
  }

  allocator_type alloc_{};
  basic_node_type NIL_{&NIL_, &NIL_, &NIL_, Color::Black};
  basic_node_type* root_ = &NIL_;
  key_compare compare_{};
};