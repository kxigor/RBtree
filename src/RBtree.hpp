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
  template <typename K, typename V, typename C, typename A>
  friend class RBtreeFriendMediator;
#endif

 private:
  struct Node;

  struct BasicNode {
    enum class Color : bool { Red, Black };

    static constexpr BasicNode* BasicNode::* another_direction(
        BasicNode* BasicNode::* direction) noexcept {
      return (direction == &BasicNode::left) ? &BasicNode::right
                                             : &BasicNode::left;
    }

    BasicNode() = delete;

    BasicNode(BasicNode* parent, BasicNode* left, BasicNode* right, Color color,
              bool nil_flag = true)
        : parent(parent),
          left(left),
          right(right),
          color(color),
          nil_flag(nil_flag) {}

    [[nodiscard]] bool is_left() const noexcept { return parent->left == this; }

    [[nodiscard]] bool is_right() const noexcept {
      return parent->right == this;
    }

    [[nodiscard]] bool is_red() const noexcept { return color == Color::Red; }

    [[nodiscard]] bool is_black() const noexcept {
      return color == Color::Black;
    }

    [[nodiscard]] bool is_nil() const noexcept { return nil_flag; }

    [[nodiscard]] bool is_not_nil() const noexcept { return !nil_flag; }

    [[nodiscard]] const key_type& get_key() const noexcept {
      return get_value().first;
    }

    [[nodiscard]] mapped_type& get_mapped() noexcept {
      return get_value().second;
    }

    [[nodiscard]] const mapped_type& get_mapped() const noexcept {
      return const_cast<BasicNode*>(this)->get_mapped();
    }

    [[nodiscard]] value_type& get_value() noexcept {
      return static_cast<Node*>(this)->val;
    }

    void replace_child_in_parent(BasicNode* new_child) noexcept {
      if (is_left()) {
        parent->left = new_child;
      } else {
        parent->right = new_child;
      }
    }

    [[nodiscard]] const value_type& get_value() const noexcept {
      return const_cast<BasicNode*>(this)->get_value();
    }

    [[nodiscard]] BasicNode* get_most_left() noexcept {
      return get_most_impl<&BasicNode::left>();
    }

    [[nodiscard]] const BasicNode* get_most_left() const noexcept {
      return const_cast<BasicNode*>(this)->get_most_left();
    }

    [[nodiscard]] BasicNode* get_most_right() noexcept {
      return get_most_impl<&BasicNode::right>();
    }

    [[nodiscard]] const BasicNode* get_most_right() const noexcept {
      return const_cast<BasicNode*>(this)->get_most_right();
    }

    template <BasicNode* BasicNode::* Direction>
    [[nodiscard]] BasicNode* get_most_impl() noexcept {
      if (is_nil()) {
        return this;
      }
      BasicNode* result = this;
      while ((result->*Direction)->is_not_nil()) {
        result = result->*Direction;
      }
      return result;
    }

    BasicNode* parent;
    BasicNode* left;
    BasicNode* right;
    Color color;
    bool nil_flag;
  };

  struct Node : BasicNode {
    template <typename... Args>
    Node(BasicNode* left, BasicNode* right, BasicNode* parent,
         typename BasicNode::Color color, bool nil_flag = false, Args&&... args)
        : BasicNode(left, right, parent, color, nil_flag),
          val(std::forward<Args>(args)...) {}
    value_type val;
  };

  using node_type = BasicNode;
  using node_allocator_type = typename std::allocator_traits<
      allocator_type>::template rebind_alloc<node_type>;
  using node_allocator_traits = std::allocator_traits<node_allocator_type>;

  using valued_node_type = Node;
  using valued_node_allocator_type = typename std::allocator_traits<
      allocator_type>::template rebind_alloc<valued_node_type>;
  using valued_node_allocator_traits =
      std::allocator_traits<valued_node_allocator_type>;

  using valued_node_pat = PropagateAssignmentTraits<valued_node_allocator_type>;
  using node_pat = PropagateAssignmentTraits<node_allocator_type>;

  using color_type = typename node_type::Color;

 public:
  /*========================= Member functions ========================*/
  RBtree() { construct_nil(); };

  ~RBtree() {
    if (!empty()) {
      clear();
    }
    if (NIL_ != nullptr) {
      destroy_nil();
    }
  }

  explicit RBtree(const Allocator& alloc) : alloc_(alloc), basic_alloc_(alloc) {
    construct_nil();
  }

  explicit RBtree(const Compare& comp, const Allocator& alloc = Allocator())
      : compare_(comp), alloc_(alloc), basic_alloc_(alloc) {
    construct_nil();
  }

  /*TODO a lot of tests*/
  RBtree(const RBtree& other)
      : alloc_(
            valued_node_allocator_traits::select_on_container_copy_construction(
                other.alloc_)),
        basic_alloc_(
            node_allocator_traits::select_on_container_copy_construction(
                other.basic_alloc_)),
        compare_(other.compare_),
        size_(other.size_) {
    construct_nil();
    copy(other);
  }

  /*TODO tests*/
  RBtree(RBtree&& other) noexcept
      : alloc_(std::move(other.alloc_)),
        basic_alloc_(std::move(other.basic_alloc_)),
        NIL_(other.NIL_),
        root_(other.root_),
        compare_(std::move(other.compare_)),
        size_(other.size_) {
    other.NIL_ = nullptr;
    other.root_ = nullptr;
    other.size_ = 0;
  }

  /*TODO tests*/
  RBtree& operator=(const RBtree& other) {
    if (valued_node_pat::is_other_allocator_copy(alloc_, other.alloc_) &&
        node_pat::is_other_allocator_copy(basic_alloc_, other.basic_alloc_)) {
      RBtree tmp(other, other.alloc_, other.basic_alloc_);
      this->swap_internal(tmp);
    } else {
      RBtree tmp(other, alloc_, basic_alloc_);
      this->swap_internal(tmp);
    }
    return *this;
  }

  /*TODO tests*/
  RBtree& operator=(RBtree&& other) {
    if (valued_node_pat::is_other_allocator_copy(alloc_, other.alloc_) &&
        node_pat::is_other_allocator_copy(basic_alloc_, other.basic_alloc_)) {
      RBtree tmp(std::move(other));
      this->swap_internal(tmp);
    } else {
      RBtree tmp(other, alloc_, basic_alloc_);
      // Переписать эту хуйню
      this->swap_internal(tmp);
    }
    return *this;
  }

 private:
  RBtree(const RBtree& other, valued_node_allocator_type alloc,
         node_allocator_type basic_alloc)
      : alloc_(alloc),
        basic_alloc_(basic_alloc),
        compare_(other.compare_),
        size_(other.size_) {
    construct_nil();
    copy(other);
  }

 public:
  [[nodiscard]] allocator_type get_allocator() const noexcept {
    return allocator_type(alloc_);
  }

  /*========================== Element access =========================*/
  mapped_type& operator[](const key_type& key) {
    auto found = find(key);
    if (found != end()) {
      return found->second;
    }
    auto [emplaced, emplace_status] = emplace(key, mapped_type{});
    assert(emplace_status == true);
    return emplaced->second;
  }

  [[nodiscard]] mapped_type& at(const key_type& key) {
    auto found = find(key);
    if (found == end()) {
      throw std::out_of_range(kOutOfRange);
    }
    return found->second;
  }

  [[nodiscard]] const mapped_type& at(const key_type& key) const {
    return const_cast<RBtree*>(this)->at(key);
  }

  /*============================ Iterators ============================*/
  [[nodiscard]] iterator begin() noexcept { return iterator(NIL_->right); }

  [[nodiscard]] const_iterator begin() const noexcept {
    return const_cast<RBtree*>(this)->begin();
  }

  [[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }

  [[nodiscard]] iterator end() noexcept { return iterator(NIL_); }

  [[nodiscard]] const_iterator end() const noexcept {
    return const_cast<RBtree*>(this)->end();
  }

  [[nodiscard]] const_iterator cend() const noexcept { return end(); }

  [[nodiscard]] reverse_iterator rbegin() noexcept {
    return std::make_reverse_iterator(begin());
  }

  [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
    return std::make_reverse_iterator(begin());
  }

  [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
    return rbegin();
  }

  [[nodiscard]] reverse_iterator rend() noexcept {
    return std::make_reverse_iterator(end());
  }

  [[nodiscard]] const_reverse_iterator rend() const noexcept {
    return std::make_reverse_iterator(end());
  }

  [[nodiscard]] const_reverse_iterator crend() const noexcept { return rend(); }

  /*============================ Capacity =============================*/
  [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

  [[nodiscard]] size_type size() const noexcept { return size_; }

  [[nodiscard]] size_type max_size() const noexcept {
    return std::numeric_limits<size_type>::max();
  }

  /*============================ Modifiers ============================*/
  void clear() noexcept { erase(begin(), end()); }

  iterator erase(const_iterator pos) noexcept {
    const iterator kNext = std::next(pos);
    erase(pos.current_node_);
    return kNext;
  }

  iterator erase(const_iterator first, const_iterator last) noexcept {
    const_iterator current = first;
    while (current != last) {
      current = erase(current);
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

  template <typename Pred>
  requires std::is_nothrow_invocable_r_v<bool, Pred,
                                         typename RBtree::value_type>
  size_type erase_if(Pred pred) noexcept {
    RBtree::size_type result = 0;
    iterator current = begin();
    iterator last = end();
    iterator next;
    while (current != last) {
      next = std::next(current);
      if (pred(*current)) {
        erase(current);
        ++result;
      }
      current = next;
    }
    return result;
  }

  std::pair<iterator, bool> insert(const value_type& value) {
    return emplace(value);
  }

  template <class P>
  std::pair<iterator, bool> insert(P&& value) {
    return emplace(std::forward<P>(value));
  }

  std::pair<iterator, bool> insert(value_type&& value) {
    return emplace(std::move(const_cast<key_type&>(value.first)),
                   std::move(value.second));
  }

  template <class... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    valued_node_type* new_node = allocate();
    construct(new_node, NIL_, NIL_, NIL_, color_type::Red, false,
              std::forward<Args>(args)...);
    return insert(static_cast<node_type*>(new_node));
  }

  template <class... Args>
  std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args) {
    auto found = find(key);
    if (found == end()) {
      return {found, false};
    }
    return emplace(key, std::forward<Args>(args)...);
  }

  /*============================== Lookup =============================*/
  [[nodiscard]] iterator lower_bound(const key_type& key) {
    return bound_impl<&RBtree::compare_greater_equal>(key);
  }

  [[nodiscard]] iterator upper_bound(const key_type& key) {
    return bound_impl<&RBtree::compare_greater>(key);
  }

  [[nodiscard]] iterator find(const key_type& key) {
    auto found = lower_bound(key);
    if (found == end() || !compare_equal(key, found->first)) {
      return end();
    }
    return found;
  }

  [[nodiscard]] const_iterator upper_bound(const key_type& key) const {
    return const_cast<RBtree*>(this)->upper_bound(key);
  }

  [[nodiscard]] const_iterator lower_bound(const key_type& key) const {
    return const_cast<RBtree*>(this)->lower_bound(key);
  }

  [[nodiscard]] const_iterator find(const key_type& key) const {
    return const_cast<RBtree*>(this)->find(key);
  }

  [[nodiscard]] bool contains(const key_type& key) const {
    return find(key) != end();
  }

  [[nodiscard]] size_type count(const key_type& key) const {
    return static_cast<size_type>(contains(key));
  }

  [[nodiscard]] std::pair<iterator, iterator> equal_range(const key_type& key) {
    return {lower_bound(key), upper_bound(key)};
  }

  [[nodiscard]] std::pair<const_iterator, const_iterator> equal_range(
      const key_type& key) const {
    return const_cast<RBtree*>(this)->equal_range(key);
  }

  /*============================ Observers ============================*/
  [[nodiscard]] key_compare key_comp() const noexcept { return compare_; }

  /*====================== Non-member functions =======================*/
  friend bool operator==(const RBtree& lhs, const RBtree& rhs) noexcept {
    return (lhs <=> rhs) == 0;
  }

  friend auto operator<=>(const RBtree& lhs, const RBtree& rhs) {
    const auto kComparePred = [&](const auto& lhs_pair, const auto& rhs_pair) {
      if (lhs.compare_less(lhs_pair.first, rhs_pair.first)) {
        return std::strong_ordering::less;
      }
      if (lhs.compare_less(rhs_pair.first, lhs_pair.first)) {
        return std::strong_ordering::greater;
      }
      return std::strong_ordering::equal;
    };
    return std::lexicographical_compare_three_way(
        lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), kComparePred);
  }

  void swap(RBtree& other) noexcept {
    if constexpr (node_allocator_traits::propagate_on_container_swap::value) {
      std::swap(this->alloc_, other.alloc_);
    }
    if constexpr (valued_node_allocator_traits::propagate_on_container_swap::
                      value) {
      std::swap(this->basic_alloc_, other.basic_alloc_);
    }
    std::swap(this->NIL_, other.NIL_);
    std::swap(this->root_, other.root_);
    std::swap(this->compare_, other.compare_);
    std::swap(this->size_, other.size_);
  }

 private:
  void copy(const RBtree& other) {
    clone_tree_nodes(other.root_);
    normalizate_nil();
  }

  void clone_tree_nodes(const node_type* other_root) {
    struct StackFrame {
      const node_type* other_node;
      node_type* parent;
    };

    // TODO alloc work
    std::vector<StackFrame> stack;
    stack.push_back({other_root, NIL_});

    while (!stack.empty()) {
      auto [other_node, parent] = stack.back();
      stack.pop_back();

      if (other_node->is_nil()) {
        continue;
      }

      valued_node_type* current = allocate();
      construct(current, parent, NIL_, NIL_, other_node->color, false,
                other_node->get_value());

      if (other_node->is_left()) {
        parent->left = current;
      } else {
        parent->right = current;
      }
      current->parent = parent;

      stack.push_back({other_node->left, current});
      stack.push_back({other_node->right, current});
    }

    root_ = NIL_->left;
  }

  void normalizate_nil() noexcept {
    NIL_->left = root_;
    NIL_->right = root_->get_most_left();
  }

  void swap_internal(RBtree& other) noexcept {
    std::swap(this->alloc_, other.alloc_);
    std::swap(this->basic_alloc_, other.basic_alloc_);
    std::swap(this->NIL_, other.NIL_);
    std::swap(this->root_, other.root_);
    std::swap(this->compare_, other.compare_);
    std::swap(this->size_, other.size_);
  }

  template <bool (RBtree::*CompareFunc)(const key_type&, const key_type&) const>
  iterator bound_impl(const key_type& key) {
    node_type* found = NIL_;
    node_type* current = root_;

    while (current->is_not_nil()) {
      if ((this->*CompareFunc)(current->get_key(), key)) {
        found = current;
        current = current->left;
      } else {
        current = current->right;
      }
    }

    return iterator(found);
  }

  /*TODO: improve codestyle*/
  std::pair<iterator, bool> insert(node_type* new_node) {
    assert(NIL_->right == root_->get_most_left());
    node_type* prev = NIL_;
    node_type* current = root_;

    while (current->is_not_nil()) {
      prev = current;
      if (compare_less(current->get_key(), new_node->get_key())) {
        current = current->right;
      } else {
        current = current->left;
      }
    }

    new_node->parent = prev;

    if (prev->is_nil()) {
      update_begin_on_insert(new_node);
      update_root(new_node);
    } else {
      if (compare_less(prev->get_key(), new_node->get_key())) {
        update_begin_on_insert(new_node);
        prev->right = new_node;
      } else if (compare_greater(prev->get_key(), new_node->get_key())) {
        update_begin_on_insert(new_node);
        prev->left = new_node;
      } else {
        annihilate(new_node);
        return {end(), false};
      }
    }

    increase_size(1);
    insert_fixup(new_node);
    return {iterator(new_node), true};
  }

  void update_begin_on_insert(node_type* insert_node) {
    if (NIL_->right->is_nil() ||
        compare_less(insert_node->get_key(), NIL_->right->get_key())) {
      NIL_->right = insert_node;
    }
  }

  void insert_fixup(node_type* current) noexcept {
    while (current->parent->is_red()) {
      if (current->parent->is_left()) {
        current = insert_fixup_impl<&node_type::left>(current);
      } else /*if (current->parent->is_right())*/ {
        current = insert_fixup_impl<&node_type::right>(current);
      }
    }
    root_->color = color_type::Black;
  }

  template <node_type* node_type::* Direction,
            node_type* node_type::* AnotherDirection =
                node_type::another_direction(Direction)>
  node_type* insert_fixup_impl(node_type* current) noexcept {
    node_type* parent = current->parent;
    node_type* grandparent = parent->parent;
    node_type* uncle = grandparent->*AnotherDirection;
    if (uncle->is_red()) {
      parent->color = color_type::Black;
      uncle->color = color_type::Black;
      grandparent->color = color_type::Red;
      current = grandparent;
    } else {
      if (current == parent->*AnotherDirection) {
        rotate_impl<Direction>(parent);
        std::swap(parent, current);
      }
      parent->color = color_type::Black;
      grandparent->color = color_type::Red;
      rotate_impl<AnotherDirection>(grandparent);
    }
    return current;
  }

  void erase(node_type* delete_node) noexcept {
    node_type* instead_node = nullptr;
    node_type* restored_node = nullptr;

    if (delete_node->left->is_nil() || delete_node->right->is_nil()) {
      instead_node = delete_node;
    } else {
      instead_node = delete_node->right->get_most_left();
    }

    const color_type kInsteadColor = instead_node->color;

    if (instead_node->left->is_not_nil()) {
      restored_node = instead_node->left;
    } else {
      restored_node = instead_node->right;
    }

    restored_node->parent = instead_node->parent;

    if (delete_node->parent->is_nil()) {
      update_root(instead_node);
    }

    if (instead_node->parent->is_nil()) {
      update_root(restored_node);
    } else {
      instead_node->replace_child_in_parent(restored_node);
    }

    if (instead_node != delete_node) {
      if (delete_node->parent->is_not_nil()) {
        delete_node->replace_child_in_parent(instead_node);
      }
      instead_node->parent = delete_node->parent;
      instead_node->left = delete_node->left;
      instead_node->right = delete_node->right;
      instead_node->left->parent = instead_node;
      instead_node->right->parent = instead_node;
      instead_node->color = delete_node->color;
    }

    if (kInsteadColor == color_type::Black) {
      erase_fixup(restored_node);
    }

    update_begin_on_erase(delete_node, instead_node, restored_node);
    annihilate(delete_node);
    decrease_size(1);
  }

  void update_begin_on_erase(node_type* delete_node, node_type* instead_node,
                             node_type* restored_node) noexcept {
    if (NIL_->right == delete_node) {
      if (instead_node == delete_node) {
        if (restored_node->is_nil()) {
          NIL_->right = delete_node->parent;
        } else {
          NIL_->right = restored_node;
        }
      } else {
        NIL_->right = instead_node;
      }
    }
  }

  void erase_fixup(node_type* restored_node) noexcept {
    while (restored_node != root_ && restored_node->is_black()) {
      if (restored_node->is_left()) {
        restored_node = erase_fixup_impl<&node_type::left>(restored_node);
      } else {
        restored_node = erase_fixup_impl<&node_type::right>(restored_node);
      }
    }
    restored_node->color = color_type::Black;
  }

  template <node_type* node_type::* Direction,
            node_type* node_type::* AnotherDirection =
                node_type::another_direction(Direction)>
  node_type* erase_fixup_impl(node_type* current) noexcept {
    node_type* parent = current->parent;
    node_type* brother = parent->*AnotherDirection;
    if (brother->is_red()) {
      brother->color = color_type::Black;
      parent->color = color_type::Red;
      rotate_impl<Direction>(parent);
      brother = parent->*AnotherDirection;
    }
    if ((brother->*Direction)->is_black() &&
        (brother->*AnotherDirection)->is_black()) {
      brother->color = color_type::Red;
      current = parent;
    } else {
      if ((brother->*AnotherDirection)->is_black()) {
        (brother->*Direction)->color = color_type::Black;
        brother->color = color_type::Red;
        rotate_impl<AnotherDirection>(brother);
        brother = parent->*AnotherDirection;
      }
      brother->color = parent->color;
      parent->color = color_type::Black;
      (brother->*AnotherDirection)->color = color_type::Black;
      rotate_impl<Direction>(parent);
      current = root_;
    }
    return current;
  }

  template <node_type* node_type::* Direction,
            node_type* node_type::* AnotherDirection =
                node_type::another_direction(Direction)>
  void rotate_impl(node_type* node) noexcept {
    node_type* child = node->*AnotherDirection;

    if (node->parent->is_nil()) {
      update_root(child);
    } else {
      node->replace_child_in_parent(child);
    }

    child->parent = node->parent;
    node->parent = child;

    node->*AnotherDirection = child->*Direction;
    if ((node->*AnotherDirection)->is_not_nil()) {
      (node->*AnotherDirection)->parent = node;
    }
    child->*Direction = node;
  }

  void update_root(node_type* new_root) noexcept {
    root_ = new_root;
    NIL_->left = new_root;
  }

  void annihilate(node_type* object) noexcept {
    assert(object->is_not_nil());
    auto* currect_pointer = static_cast<valued_node_type*>(object);
    destroy(currect_pointer);
    deallocate(currect_pointer);
  }

  valued_node_type* allocate() {
    return valued_node_allocator_traits::allocate(alloc_, 1);
  }

  void deallocate(valued_node_type* object) noexcept {
    valued_node_allocator_traits::deallocate(alloc_, object, 1);
  }

  template <typename... Args>
  void construct(valued_node_type* object, Args&&... args) {
    valued_node_allocator_traits::construct(alloc_, object,
                                            std::forward<Args>(args)...);
  }

  void destroy(valued_node_type* object) noexcept {
    valued_node_allocator_traits::destroy(alloc_, object);
  }

  [[nodiscard]] bool compare_less(const key_type& lhs,
                                  const key_type& rhs) const {
    return compare_(lhs, rhs);
  }

  [[nodiscard]] bool compare_less_equal(const key_type& lhs,
                                        const key_type& rhs) const {
    return !compare_(rhs, lhs);
  }

  [[nodiscard]] bool compare_greater(const key_type& lhs,
                                     const key_type& rhs) const {
    return compare_(rhs, lhs);
  }

  [[nodiscard]] bool compare_greater_equal(const key_type& lhs,
                                           const key_type& rhs) const {
    return !compare_(lhs, rhs);
  }

  [[nodiscard]] bool compare_equal(const key_type& lhs,
                                   const key_type& rhs) const {
    return compare_less_equal(lhs, rhs) && compare_greater_equal(lhs, rhs);
  }

  void increase_size(std::size_t offset) noexcept { size_ += offset; }

  void decrease_size(std::size_t offset) noexcept { size_ -= offset; }

  void construct_nil() {
    NIL_ = node_allocator_traits::allocate(basic_alloc_, 1);
    std::construct_at(NIL_, NIL_, NIL_, NIL_, color_type::Black, true);
    root_ = NIL_;
  }

  void destroy_nil() noexcept {
    std::destroy_at(NIL_);
    node_allocator_traits::deallocate(basic_alloc_, NIL_, 1);
  }

  valued_node_allocator_type alloc_{};
  node_allocator_type basic_alloc_{};

  node_type* NIL_{};
  node_type* root_{};
  key_compare compare_{};
  size_type size_{};
};

template <class Key, class T, class Compare, class Allocator>
template <bool IsConst>
class RBtree<Key, T, Compare, Allocator>::Iterator {
  /*======================== Usings and Structures =========================*/
  using rbtree = RBtree<Key, T, Compare, Allocator>;
  using node_type = rbtree::node_type;
  using valued_node_type = rbtree::valued_node_type;

 public:
  using difference_type = ptrdiff_t;
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = rbtree::value_type;
  using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;
  using reference = std::conditional_t<IsConst, const value_type&, value_type&>;

  /*============================ Constructors ==============================*/
  Iterator() = default;
  ~Iterator() = default;

  explicit Iterator(node_type* node) noexcept : current_node_(node) {}

  Iterator(const Iterator& /*unused*/) noexcept = default;

  Iterator(Iterator&& /*unused*/) noexcept = default;

  Iterator& operator=(const Iterator& /*unused*/) noexcept = default;

  Iterator& operator=(Iterator&& /*unused*/) noexcept = default;
  /*============================== Operators ===============================*/

  Iterator& operator++() noexcept {
    return operator_unary_step_impl<&node_type::left>();
  }

  Iterator& operator--() noexcept {
    return operator_unary_step_impl<&node_type::right>();
  }

  Iterator operator++(int) noexcept {
    Iterator result = *this;
    ++*this;
    return result;
  }

  Iterator operator--(int) noexcept {
    Iterator result = *this;
    --*this;
    return result;
  }

  reference operator*() const noexcept {
    return const_cast<reference>(current_node_->get_value());
  }

  pointer operator->() const noexcept {
    return const_cast<pointer>(std::addressof(current_node_->get_value()));
  }

  bool operator==(const Iterator& other) const noexcept {
    return current_node_ == other.current_node_;
  }

  bool operator!=(const Iterator& other) const noexcept {
    return !(*this == other);
  }

  operator Iterator<true>() const noexcept {
    return Iterator<true>(current_node_);
  }

 private:
  friend RBtree;

#ifdef DEBUG_
  template <typename K, typename V, typename C, typename A>
  friend class RBtreeFriendMediator;
#endif

  operator Iterator<false>() const noexcept {
    return Iterator<false>(current_node_);
  }

  template <node_type* node_type::* Direction,
            node_type* node_type::* AnotherDirection =
                node_type::another_direction(Direction)>
  Iterator& operator_unary_step_impl() noexcept {
    if ((current_node_->*AnotherDirection)->is_not_nil()) {
      current_node_ = current_node_->*AnotherDirection;
      current_node_ = current_node_->template get_most_impl<Direction>();
      return *this;
    }
    slide_up_while_not_impl<AnotherDirection>();
    return *this;
  }

  template <node_type* node_type::* Direction>
  void slide_up_while_not_impl() noexcept {
    while (current_node_->parent->*Direction == current_node_ &&
           current_node_->parent->is_not_nil() && current_node_->is_not_nil()) {
      current_node_ = current_node_->parent;
    }
    current_node_ = current_node_->parent;
  }

  /*================================ Fields ================================*/
  node_type* current_node_{nullptr};
};

namespace std {
template <class Key, class T, class Compare, class Allocator>
void swap(RBtree<Key, T, Compare, Allocator>& lhs,
          RBtree<Key, T, Compare, Allocator>& rhs) noexcept {
  lhs.swap(rhs);
}
}  // namespace std