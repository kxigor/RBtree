#include <memory>

template <typename Alloc>
class propagate_assignment_traits {
  using allocator_traits = std::allocator_traits<Alloc>;
  using copy_type =
      typename allocator_traits::propagate_on_container_copy_assignment;
  using move_type =
      typename allocator_traits::propagate_on_container_move_assignment;

 public:
  static constexpr bool is_other_allocator_copy(const Alloc& alloc,
                                                const Alloc& other_alloc) {
    return impl<copy_type>(alloc, other_alloc);
  }
  static constexpr bool is_other_allocator_move(const Alloc& alloc,
                                                const Alloc& other_alloc) {
    return impl<move_type>(alloc, other_alloc);
  }

 private:
  template <typename PropageteType>
  static constexpr bool impl(const Alloc& alloc, const Alloc& other_alloc) {
    return PropageteType::value ||
           (!allocator_traits::is_always_equal::value && alloc != other_alloc);
  }
};
