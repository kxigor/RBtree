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

  struct BasicNode {
    enum class Color : bool { red, black };

    BasicNode* left;
    BasicNode* right;
    BasicNode* parent;

    Color color;
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

  RBtree() {}

  template <class... Args>
  std::pair<iterator, bool> emplace(Args&&... args) {

  }

 private:
  void left_rotate() {

  }

  void right_rotate() {
    
  }

  node_type* allocate() { return node_allocator_traits::allocate(alloc_, 1); }

  template <typename... Args>
  void construct(node_type* object, Args&&... args) {
    node_allocator_traits::construct(alloc_, object,
                                     std::forward<Args>(args)...);
  }

  allocator_type alloc_{};
  basic_node_type NIL_{&NIL_, &NIL_, &NIL_, Color::black};
  basic_node_type* root_ = &NIL_;
};