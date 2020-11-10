#ifndef HANDLE_H_INCLUDE_GUARD
#define HANDLE_H_INCLUDE_GUARD
// Handle.h

#include <type_traits> // aligned_storage

namespace lib {

template<typename Body, std::size_t Size = 4 * sizeof(void*)>
class Handle
{
  static_assert( Size >= sizeof(void*),
                 "'Size' must be large enough to hold a pointer" );
  // either opaque pointer, or in-place body.
  std::aligned_storage_t<Size> storage_; 
public:
  // Alias for the type of the body handled by this class.
  using body_type = Body;

  // Create a 'Handle' object handling a 'Body' object, created using its
  // default constructor. Note that the body is stored in-place if it fits
  // the storage provided by this object; it is dynamically allocated otherwise.
  Handle();

  // Left out because slideware: generic forwarding constructors.

  /* Rule of 5 */
  Handle(Handle const& other);
  Handle(Handle && other) noexcept;

  ~Handle() noexcept;

  Handle& operator = (Handle const& other);
  Handle& operator = (Handle && other) noexcept;

  // Return a pointer to the body handled by this object, enforcing const-correctness.
  // The behaviour is undefined if this object is in a moved-from state.
  body_type const* operator -> () const noexcept;
  body_type      * operator -> ()       noexcept;
};

} // namespace lib

#endif // HANDLE_H_INCLUDE_GUARD
