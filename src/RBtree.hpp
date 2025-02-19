#pragma once

#define DEBUG_

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

#include "PropagateAssignmentTraits.hpp"

template <class Key, class T, class Compare = std::less<Key>,
          class Allocator = std::allocator<std::pair<const Key, T>>>
class RBtree {
  static constexpr const char* kBadEmplaceMessage = "Bad Emplace";
  static constexpr const char* kOutOfRange = "Missing element";

  template <bool IsConst>
  class Iterator;

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
  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

#ifdef DEBUG_
  class RBtreeVisualizer;
#endif

 private:
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

    const key_type& get_key() const { return get_value().first; }

    mapped_type& get_mapped() { return get_value().second; }

    const mapped_type& get_mapped() const {
      return const_cast<BasicNode*>(this)->get_mapped();
    }

    value_type& get_value() {
      assert(dynamic_cast<const Node*>(this) != nullptr);
      return static_cast<Node*>(this)->val;
    }

    const value_type& get_value() const {
      return const_cast<BasicNode*>(this)->get_value();
    }

    static constexpr BasicNode* BasicNode::* another_direction(
        BasicNode* BasicNode::* direction) {
      return (direction == &BasicNode::left) ? &BasicNode::right
                                             : &BasicNode::left;
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

 public:
  /*========================= Member functions ========================*/
  RBtree() = default;

  ~RBtree() { clear(); }

  allocator_type get_allocator() const noexcept { return allocator_type(); }

  /*========================== Element access =========================*/
  mapped_type& operator[](const key_type& key) {
    auto found = find(key);
    if (found != end()) {
      return found->second;
    }
    auto [emplaced, empalce_status] = emplace(key, mapped_type{});
    assert(empalce_status == true);
    return emplaced->second;
  }

  mapped_type& at(const key_type& key) {
    auto found = find(key);
    if (found == end()) {
      throw std::out_of_range(kOutOfRange);
    }
    return found->second;
  }

  const mapped_type& at(const key_type& key) const {
    return const_cast<RBtree*>(this)->at(key);
  }

  /*============================ Iterators ============================*/
  /*TODO: Complexity Constant*/
  iterator begin() noexcept {
    basic_node_type* most_left = root_;
    while (!is_nil(most_left->left)) {
      most_left = most_left->left;
    }
    return construct_iterator(most_left);
  }

  const_iterator begin() const noexcept {
    return const_cast<RBtree*>(this)->begin();
  }

  const_iterator cbegin() const noexcept { return begin(); }

  iterator end() noexcept { return construct_iterator(&NIL_); }

  const_iterator end() const noexcept {
    return const_cast<RBtree*>(this)->end();
  }

  const_iterator cend() const noexcept { return end(); }

  reverse_iterator rbegin() noexcept {
    return std::make_reverse_iterator(begin());
  }

  const_reverse_iterator rbegin() const noexcept {
    return std::make_reverse_iterator(begin());
  }

  const_reverse_iterator crbegin() const noexcept { return rbegin(); }

  reverse_iterator rend() noexcept { return std::make_reverse_iterator(end()); }

  const_reverse_iterator rend() const noexcept {
    return std::make_reverse_iterator(end());
  }

  const_reverse_iterator crend() const noexcept { return rend(); }

  /*============================ Capacity =============================*/
  bool empty() const noexcept { return size_ == 0; }

  size_type size() const noexcept { return size_; }

  size_type max_size() const noexcept {
    return std::numeric_limits<size_type>::max();
  }

  /*============================ Modifiers ============================*/
  void clear() noexcept { clear_impl(); }

  iterator erase(const_iterator pos) noexcept {
    iterator next = std::next(pos);
    erase(pos.current_node_);
    return next;
  }

  iterator erase(const_iterator first, const_iterator last) noexcept {
    const_iterator current = first;
    const_iterator next;

    while (current != last) {
      next = std::next(current);
      erase(current);
      current = next;
    }
    return current;
  }

  size_type erase(const key_type& key) {
    auto pos = find(key);
    if (pos == end()) {
      return 0;
    }
    erase(pos);
    return 1;
  }

  std::pair<iterator, bool> insert(const value_type& value) {
    return emplace(value);
  }

  template <class P>
  std::pair<iterator, bool> insert(P&& value) {
    return emplace(std::move(value));
  }

  std::pair<iterator, bool> insert(value_type&& value) {
    return emplace(std::move(const_cast<key_type&>(value.first)),
                   std::move(value.second));
  }

  template <class... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    node_type* new_node = allocate();
    construct(new_node, &NIL_, &NIL_, &NIL_, Color::Red,
              std::forward<Args>(args)...);
    return insert(static_cast<basic_node_type*>(new_node));
  }

  template <class... Args>
  std::pair<iterator, bool> try_emplace(const key_type& k, Args&&... args) {
    auto found = find(k);
    if (found == end()) {
      return {found, false};
    }
    return emplace(k, std::forward<Args>(args)...);
  }

  /*============================== Lookup =============================*/
  iterator lower_bound(const key_type& key) {
    return bound_impl<&RBtree::compare_greater_equal>(key);
  }

  iterator upper_bound(const key_type& key) {
    return bound_impl<&RBtree::compare_greater>(key);
  }

  iterator find(const key_type& key) {
    auto found = lower_bound(key);
    if (found == end() || !compare_equal(key, found->first)) {
      return end();
    }
    return found;
  }

  const_iterator upper_bound(const key_type& key) const {
    return const_cast<RBtree*>(this)->upper_bound(key);
  }

  const_iterator lower_bound(const key_type& key) const {
    return const_cast<RBtree*>(this)->lower_bound(key);
  }

  const_iterator find(const key_type& key) const {
    return const_cast<RBtree*>(this)->find(key);
  }

  bool contains(const key_type& key) const { return find(key) != end(); }

  size_type count(const key_type& key) const {
    return static_cast<size_type>(contains(key));
  }

  std::pair<iterator, iterator> equal_range(const key_type& key) {
    return {lower_bound(key), upper_bound(key)};
  }

  std::pair<const_iterator, const_iterator> equal_range(
      const key_type& key) const {
    return const_cast<RBtree*>(this)->equal_range(key);
  }

  /*============================ Observers ============================*/
  key_compare key_comp() const noexcept { return compare_; }

  /*====================== Non-member functions =======================*/
  friend bool operator==(const RBtree& lhs, const RBtree& rhs) {
    return (lhs <=> rhs) == 0;
  }
  /*TODO: improve codestyle*/
  friend auto operator<=>(const RBtree& lhs, const RBtree& rhs) {
    return std::lexicographical_compare_three_way(
        lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
        [&](const auto& l, const auto& r) {
          if (lhs.compare_less(l.first, r.first)) {
            return std::strong_ordering::less;
          }
          if (lhs.compare_less(r.first, l.first)) {
            return std::strong_ordering::greater;
          }
          return std::strong_ordering::equal;
        });
  }

  /*TODO code*/
  friend void swap(const RBtree& lhs, const RBtree& rhs) {
    (void)lhs, (void)rhs;
  }

  /*TODO tests*/
  template <typename Pred>
    requires std::is_nothrow_invocable_r_v<bool, Pred,
                                           typename RBtree::value_type>
  friend size_type erase_if(RBtree& tree, Pred pred) noexcept {
    RBtree::size_type result = 0;
    iterator current = tree.begin();
    iterator last = tree.end();
    iterator next;
    while (current != last) {
      next = std::next(current);
      if (pred(*current)) {
        tree.erase(current);
        ++result;
      }
      current = next;
    }
    return result;
  }

 private:
  template <bool (RBtree::*compare)(const key_type&, const key_type&) const>
  iterator bound_impl(const key_type& key) {
    basic_node_type* found = &NIL_;
    basic_node_type* current = root_;

    while (!is_nil(current)) {
      if ((this->*compare)(current->get_key(), key)) {
        found = current;
        current = current->left;
      } else {
        current = current->right;
      }
    }

    return construct_iterator(found);
  }

  /*TODO: improve codestyle*/
  std::pair<iterator, bool> insert(basic_node_type* new_node) {
    basic_node_type* prev = &NIL_;
    basic_node_type* current = root_;

    while (!is_nil(current)) {
      prev = current;
      if (compare_less(current->get_key(), new_node->get_key())) {
        current = current->right;
      } else {
        current = current->left;
      }
    }

    new_node->parent = prev;

    if (is_nil(prev)) {
      update_root(new_node);
    } else {
      if (compare_less(prev->get_key(), new_node->get_key())) {
        prev->right = new_node;
      } else if (compare_greater(prev->get_key(), new_node->get_key())) {
        prev->left = new_node;
      } else {
        annihilate(new_node);
        return {end(), false};
      }
    }

    increase_size(1);
    insert_fixup(new_node);
    return {construct_iterator(new_node), true};
  }

  void insert_fixup(basic_node_type* current) noexcept {
    while (current->parent->is_red()) {
      if (current->parent->is_left()) {
        current = insert_fixup_impl<&basic_node_type::left>(current);
      } else /*if (current->parent->is_right())*/ {
        current = insert_fixup_impl<&basic_node_type::right>(current);
      }
    }
    root_->color = Color::Black;
  }

  template <basic_node_type* basic_node_type::* direction,
            basic_node_type* basic_node_type::* another_direction =
                basic_node_type::another_direction(direction)>
  basic_node_type* insert_fixup_impl(basic_node_type* current) noexcept {
    basic_node_type* parent = current->parent;
    basic_node_type* grandparent = parent->parent;
    basic_node_type* uncle = grandparent->*another_direction;
    if (uncle->is_red()) {
      parent->color = Color::Black;
      uncle->color = Color::Black;
      grandparent->color = Color::Red;
      current = grandparent;
    } else {
      if (current == parent->*another_direction) {
        rotate_impl<direction>(parent);
        std::swap(parent, current);
      }
      parent->color = Color::Black;
      grandparent->color = Color::Red;
      rotate_impl<another_direction>(grandparent);
    }
    return current;
  }

  void erase(basic_node_type* delete_node) noexcept {
    basic_node_type* instead_node = nullptr;
    basic_node_type* restored_node = nullptr;
    if (is_nil(delete_node->left) || is_nil(delete_node->right)) {
      instead_node = delete_node;
    } else {
      instead_node = get_minimum(delete_node->right);
    }

    if (!is_nil(instead_node->left)) {
      restored_node = instead_node->left;
    } else {
      restored_node = instead_node->right;
    }

    restored_node->parent = instead_node->parent;

    if (is_nil(delete_node->parent)) {
      update_root(instead_node);
    }

    if (is_nil(instead_node->parent)) {
      update_root(restored_node);
    } else {
      replace_child_in_parent(instead_node, restored_node);
    }

    if (instead_node != delete_node) {
      replace_child_in_parent(delete_node, instead_node);
      instead_node->parent = delete_node->parent;
      instead_node->left = delete_node->left;
      instead_node->right = delete_node->right;
      instead_node->left->parent = instead_node;
      instead_node->right->parent = instead_node;
    }

    if (instead_node->is_black()) {
      erase_fixup(restored_node);
    }

    annihilate(delete_node);
    decrease_size(1);
  }

  void replace_child_in_parent(basic_node_type* current,
                               basic_node_type* new_child) {
    if (current->is_left()) {
      current->parent->left = new_child;
    } else {
      current->parent->right = new_child;
    }
  }

  basic_node_type* get_minimum(basic_node_type* current) {
    while (!is_nil(current->left)) {
      current = current->left;
    }
    return current;
  }

  void erase_fixup(basic_node_type* restored_node) noexcept {
    while (restored_node != root_ && restored_node->is_black()) {
      if (restored_node->is_left()) {
        restored_node = erase_fixup_impl<&basic_node_type::left>(restored_node);
      } else {
        restored_node =
            erase_fixup_impl<&basic_node_type::right>(restored_node);
      }
    }
    restored_node->color = Color::Black;
  }

  template <basic_node_type* basic_node_type::* direction,
            basic_node_type* basic_node_type::* another_direction =
                basic_node_type::another_direction(direction)>
  basic_node_type* erase_fixup_impl(basic_node_type* current) noexcept {
    basic_node_type* parent = current->parent;
    basic_node_type* brother = parent->*another_direction;
    if (brother->is_red()) {
      brother->color = Color::Black;
      parent->color = Color::Red;
      rotate_impl<direction>(parent);
      brother = parent->*another_direction;
    }
    if ((brother->*direction)->is_black() &&
        (brother->*another_direction)->is_black()) {
      brother->color = Color::Red;
      current = parent;
    } else {
      if ((brother->*another_direction)->is_black()) {
        (brother->*direction)->color = Color::Black;
        brother->color = Color::Red;
        rotate_impl<another_direction>(brother);
        brother = parent->*another_direction;
      }
      brother->color = parent->color;
      parent->color = Color::Black;
      (brother->*another_direction)->color = Color::Black;
      rotate_impl<direction>(parent);
      current = root_;
    }
    return current;
  }

  template <basic_node_type* basic_node_type::* direction,
            basic_node_type* basic_node_type::* another_direction =
                basic_node_type::another_direction(direction)>
  void rotate_impl(basic_node_type* node) noexcept {
    basic_node_type* child = node->*another_direction;

    child->parent = node->parent;
    node->parent = child;

    if (is_nil(child->parent)) {
      update_root(child);
    } else {
      if (child->parent->left == node) {
        child->parent->left = child;
      } else {
        child->parent->right = child;
      }
    }

    node->*another_direction = child->*direction;
    if (!is_nil(node->*another_direction)) {
      (node->*another_direction)->parent = node;
    }
    child->*direction = node;
  }

  void clear_impl() noexcept {
    basic_node_type* current = root_;

    while (!is_nil(current)) {
      while (!is_nil(current->left) || !is_nil(current->right)) {
        if (!is_nil(current->left)) {
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

  void update_root(basic_node_type* new_root) noexcept {
    root_ = new_root;
    NIL_.left = new_root;
  }

  void annihilate(basic_node_type* object) noexcept {
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

  iterator construct_iterator(basic_node_type* node) noexcept {
    return {node, &NIL_};
  }

  bool compare_less(const key_type& lhs, const key_type& rhs) const {
    return compare_(lhs, rhs);
  }

  bool compare_less_equal(const key_type& lhs, const key_type& rhs) const {
    return !compare_(rhs, lhs);
  }

  bool compare_greater(const key_type& lhs, const key_type& rhs) const {
    return compare_(rhs, lhs);
  }

  bool compare_greater_equal(const key_type& lhs, const key_type& rhs) const {
    return !compare_(lhs, rhs);
  }

  bool compare_equal(const key_type& lhs, const key_type& rhs) const {
    return compare_less_equal(lhs, rhs) && compare_greater_equal(lhs, rhs);
  }

  void increase_size(std::size_t offset) noexcept { size_ += offset; }

  void decrease_size(std::size_t offset) noexcept { size_ -= offset; }

  bool is_nil(basic_node_type* node) const noexcept { return node == &NIL_; }

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
    return operator_unary_step_impl<&basic_node_type::left>();
  }

  Iterator& operator--() {
    return operator_unary_step_impl<&basic_node_type::right>();
  }

  Iterator operator++(int) {
    Iterator result = *this;
    ++*this;
    return result;
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

  operator Iterator<true>() const {
    return Iterator<true>(current_node_, NIL_);
  }

 private:
  friend RBtree;

  operator Iterator<false>() const {
    return Iterator<false>(current_node_, NIL_);
  }

  template <basic_node_type* basic_node_type::* direction,
            basic_node_type* basic_node_type::* another_direction =
                basic_node_type::another_direction(direction)>
  Iterator& operator_unary_step_impl() {
    if (current_node_->*another_direction != NIL_) {
      current_node_ = current_node_->*another_direction;
      slide_fully_impl<direction>();
      return *this;
    }
    slide_up_while_not_impl<another_direction>();
    return *this;
  }

  template <basic_node_type* basic_node_type::* direction>
  void slide_fully_impl() {
    while (current_node_->*direction != NIL_) {
      current_node_ = current_node_->*direction;
    }
  }

  template <basic_node_type* basic_node_type::* direction>
  void slide_up_while_not_impl() {
    while (current_node_->parent->*direction == current_node_ &&
           current_node_ != NIL_) {
      current_node_ = current_node_->parent;
    }
    current_node_ = current_node_->parent;
  }

  /*================================ Fields ================================*/
  basic_node_type* current_node_{nullptr};
  basic_node_type* NIL_{nullptr};
};

#ifdef DEBUG_
#include "RBtreeVizualizer.hpp"
#endif