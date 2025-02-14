#include <cassert>
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

  template <bool IsConst>
  class Iterator;

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  struct Node;

  struct BasicNode {
    enum class Color : bool { Red, Black };

    BasicNode() = delete;
    BasicNode(BasicNode* left, BasicNode* right, BasicNode* parent, Color color)
        : left(left), right(right), parent(parent), color(color) {}

    virtual ~BasicNode() = default;

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
      return get_value().first;
    }

    mapped_type& get_mapped() {
      assert(dynamic_cast<Node*>(this) != nullptr);
      return get_value().second;
    }

    const mapped_type& get_mapped() const {
      assert(dynamic_cast<const Node*>(this) != nullptr);
      return const_cast<BasicNode*>(this)->get_mapped();
    }

    value_type& get_value() { return static_cast<Node*>(this)->val; }

    const value_type& get_value() const {
      return const_cast<BasicNode*>(this)->get_value();
    }
  };

  struct Node : BasicNode {
    template <typename... Args>
    Node(BasicNode* left, BasicNode* right, BasicNode* parent,
         typename BasicNode::Color color, Args&&... args)
        : BasicNode(left, right, parent, color),
          val(std::forward<Args>(args)...) {}
    value_type val;
  };

  using basic_node_type = BasicNode;
  using node_type = Node;
  using node_allocator_type = typename std::allocator_traits<
      allocator_type>::template rebind_alloc<node_type>;
  using node_allocator_traits = std::allocator_traits<node_allocator_type>;

  using Color = typename basic_node_type::Color;

  RBtree() = default;
  ~RBtree() { clear(); }

  /*============================ Iterators ============================*/
  iterator begin() {
    basic_node_type* most_left = root_;
    while (most_left->left != &NIL_) {
      most_left = most_left->left;
    }
    return {most_left, &NIL_};
  }

  iterator end() { return {&NIL_, &NIL_}; }
  /*============================ Capacity =============================*/
  size_type size() const { return size_; }

  /*============================ Modifiers ============================*/
  void clear() noexcept { clear_impl(); }

  iterator erase(iterator pos) noexcept;
  iterator erase(const_iterator pos) noexcept;
  iterator erase(iterator first, iterator last) noexcept;

  // template <class P>
  // std::pair<iterator, bool> insert(P&& value) {
  //   node_type* new_node = allocate();
  //   construct(new_node, std::forward<P>(value));
  //   return insert(static_cast<basic_node_type*>(new_node));
  // }

  std::pair<iterator, bool> insert(value_type&& value) {
    node_type* new_node = allocate();
    construct(new_node, &NIL_, &NIL_, &NIL_, Color::Red,
              std::move(const_cast<key_type&>(value.first)),
              std::move(value.second));
    return insert(static_cast<basic_node_type*>(new_node));
  }

 private:
  std::pair<iterator, bool> insert(basic_node_type* z) {
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
      } else if (compare_(z->get_key(), y->get_key())) {
        y->left = z;
      } else {
        annihilate(z);
        return {end(), false};
      }
    }

    ++size_;
    insert_fixup(z);
    return {{z, &NIL_}, true};
  }

  void insert_fixup(basic_node_type* current) {
    while (current->parent->is_red()) {
      if (current->parent->is_left()) {
        current = insert_fixup_left_case(current);
      } else /*if (current->parent->is_right())*/ {
        current = insert_fixup_right_case(current);
      }
    }
    root_->color = Color::Black;
  }

  basic_node_type* insert_fixup_left_case(basic_node_type* current) {
    return insert_fixup_impl<&basic_node_type::left, &basic_node_type::right>(
        current);
  }

  basic_node_type* insert_fixup_right_case(basic_node_type* current) {
    return insert_fixup_impl<&basic_node_type::right, &basic_node_type::left>(
        current);
  }

  template <basic_node_type* basic_node_type::*direction1,
            basic_node_type* basic_node_type::*direction2>
  basic_node_type* insert_fixup_impl(basic_node_type* current) {
    basic_node_type* parent = current->parent;
    basic_node_type* grandparent = parent->parent;
    basic_node_type* uncle = grandparent->*direction2;
    if (uncle->is_red()) {
      parent->color = Color::Black;
      uncle->color = Color::Black;
      grandparent->color = Color::Red;
      current = grandparent;
    } else {
      if (current == parent->*direction2) {
        rotate_impl<direction2, direction1>(parent);
        std::swap(parent, current);
      }
      parent->color = Color::Black;
      grandparent->color = Color::Red;
      rotate_impl<direction1, direction2>(grandparent);
    }
    return current;
  }

  template <basic_node_type* basic_node_type::*direction1,
            basic_node_type* basic_node_type::*direction2>
  void rotate_impl(basic_node_type* node) {
    basic_node_type* child = node->*direction1;

    child->parent = node->parent;
    node->parent = child;

    if (child->parent == &NIL_) {
      update_root(child);
    } else {
      if (child->parent->left == node) {
        child->parent->left = child;
      } else {
        child->parent->right = child;
      }
    }

    node->*direction1 = child->*direction2;
    if (node->*direction1 != &NIL_) {
      (node->*direction1)->parent = node;
    }
    child->*direction2 = node;
  }

  bool compare_equal(const key_type& lhs, const key_type& rhs) {
    return !compare_(lhs, rhs) && !compare_(rhs, lhs);
  }

  void update_root(basic_node_type* new_root) {
    root_ = new_root;
    NIL_.left = new_root;
  }

  void clear_impl() noexcept {
    basic_node_type* current = root_;

    while (current != &NIL_) {
      while (current->left != &NIL_ || current->right != &NIL_) {
        if (current->left != &NIL_) {
          current = current->left;
        } else {
          current = current->right;
        }
      }
      basic_node_type* next = current->parent;
      annihilate(current);
      if (next->left == current) {
        next->left = &NIL_;
      } else {
        next->right = &NIL_;
      }
      current = next;
    }

    update_root(&NIL_);
    size_ = 0;
  }

  void annihilate(basic_node_type* object) {
    assert(dynamic_cast<node_type*>(object) != nullptr);
    node_type* currect_pointer = dynamic_cast<node_type*>(object);
    destroy(currect_pointer);
    deallocate(currect_pointer);
  }

  node_type* allocate() { return node_allocator_traits::allocate(alloc_, 1); }

  void deallocate(node_type* object) noexcept {
    node_allocator_traits::deallocate(alloc_, object, 1);
  }

  template <typename... Args>
  void construct(node_type* object, Args&&... args) {
    node_allocator_traits::construct(alloc_, object,
                                     std::forward<Args>(args)...);
  }

  void destroy(node_type* object) noexcept {
    node_allocator_traits::destroy(alloc_, object);
  }

  node_allocator_type alloc_{};
  basic_node_type NIL_{&NIL_, &NIL_, &NIL_, Color::Black};
  basic_node_type* root_ = &NIL_;
  key_compare compare_{};
  size_type size_{};
};

template <class Key, class T, class Compare, class Allocator>
template <bool IsConst>
class RBtree<Key, T, Compare, Allocator>::Iterator {
  /*======================== Usings and Structures =========================*/
  friend class List;
  using rbtree = RBtree<Key, T, Compare, Allocator>;
  using basic_node_type = rbtree::basic_node_type;
  using node_type = rbtree::node_type;

 public:
  using difference_type = ptrdiff_t;
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = rbtree::value_type;
  using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;
  using reference = std::conditional_t<IsConst, const value_type&, value_type&>;

  /*============================ Constructors ==============================*/
  Iterator() = default;

  Iterator(basic_node_type* node, basic_node_type* nil)
      : current_node_(node), NIL_(nil) {}

  Iterator(const Iterator& /*unused*/) = default;

  /*============================== Operators ===============================*/
  Iterator& operator=(const Iterator& /*unused*/) = default;

  Iterator& operator++() {
    if (current_node_->right != NIL_) {
      current_node_ = current_node_->right;
      slide_left_fully();
      return *this;
    }

    slide_up_while_not_left();

    return *this;
  }

  Iterator operator++(int) {
    Iterator result = *this;
    ++*this;
    return result;
  }

  Iterator& operator--() {
    if (current_node_->left != NIL_) {
      current_node_ = current_node_->left;
      slide_right_fully();
      return *this;
    }

    slide_up_while_not_right();

    return *this;
  }

  Iterator operator--(int) {
    Iterator result = *this;
    --*this;
    return result;
  }

  reference operator*() const {
    return const_cast<reference>(current_node_->get_value());
  }

  pointer operator->() const {
    return const_cast<pointer>(std::addressof(current_node_->get_value()));
  }

  bool operator==(const Iterator& other) const {
    return current_node_ == other.current_node_;
  }

  bool operator!=(const Iterator& other) const { return !(*this == other); }

  operator Iterator<true>() const { return Iterator<true>(current_node_); }

 private:
  void slide_left_fully() {
    while (current_node_->left != NIL_) {
      current_node_ = current_node_->left;
    }
  }

  void slide_right_fully() {
    while (current_node_->right != NIL_) {
      current_node_ = current_node_->right;
    }
  }

  void slide_up_while_not_left() {
    basic_node_type* prev_node = nullptr;
    while (current_node_->is_right() && !is_root()) {
      prev_node = current_node_;
      current_node_ = current_node_->parent;
    }
    prev_node = current_node_;
    current_node_ = current_node_->parent;
    if (current_node_->right == prev_node && is_root()) {
      current_node_ = NIL_;
    }
  }

  void slide_up_while_not_right() {
    basic_node_type* prev_node = nullptr;
    while (current_node_->is_left() && !is_root()) {
      prev_node = current_node_;
      current_node_ = current_node_->parent;
    }
    prev_node = current_node_;
    current_node_ = current_node_->parent;
    if (current_node_->left == prev_node && is_root()) {
      current_node_ = NIL_;
    }
  }

  bool is_root() { return current_node_->parent == NIL_; }

  /*================================ Fields ================================*/
  basic_node_type* current_node_{nullptr};
  basic_node_type* NIL_{nullptr};
};