#include <memory>

template <typename T, bool IsAlwaysEqual = false,
          bool SelectOnCopyConstruction = false,
          bool PropagateOnCopyAssignment = false,
          bool PropagateOnMoveAssignment = false, bool PropagateOnSwap = true>
struct PolicyAwareAllocator {
  static constexpr const int kDefaultId = 0;

  using value_type = T;

  PolicyAwareAllocator select_on_container_copy_construction() const {
    if constexpr (SelectOnCopyConstruction) {
      return PolicyAwareAllocator(id);
    } else {
      return PolicyAwareAllocator();
    }
  }

  using is_always_equal = std::bool_constant<IsAlwaysEqual>;
  using propagate_on_container_copy_assignment =
      std::bool_constant<PropagateOnCopyAssignment>;
  using propagate_on_container_move_assignment =
      std::bool_constant<PropagateOnMoveAssignment>;
  using propagate_on_container_swap = std::bool_constant<PropagateOnSwap>;

  PolicyAwareAllocator() = default;
  PolicyAwareAllocator(int id) : id(id) {}

  template <typename U>
  struct rebind {
    using other =
        PolicyAwareAllocator<U, IsAlwaysEqual, SelectOnCopyConstruction,
                             PropagateOnCopyAssignment,
                             PropagateOnMoveAssignment, PropagateOnSwap>;
  };

  template <typename U>
  PolicyAwareAllocator(
      const PolicyAwareAllocator<
          U, IsAlwaysEqual, SelectOnCopyConstruction, PropagateOnCopyAssignment,
          PropagateOnMoveAssignment, PropagateOnSwap>& other)
      : id(other.id) {}

  T* allocate(size_t n) {
    return static_cast<T*>(::operator new(n * sizeof(T)));
  }

  void deallocate(T* p, size_t) { ::operator delete(p); }

  bool operator==(const PolicyAwareAllocator& other) const {
    if constexpr (IsAlwaysEqual) {
      return true;
    } else {
      return id == other.id;
    }
  }
  bool operator!=(const PolicyAwareAllocator& other) const {
    return !(*this == other);
  }

  int id{kDefaultId};
};