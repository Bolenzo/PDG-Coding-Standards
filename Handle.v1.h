#ifndef HANDLE_H_INCLUDE_GUARD
#define HANDLE_H_INCLUDE_GUARD
// Handle.h

#include <memory> // unique_ptr

namespace lib {

template<typename Body>
class Handle
{
  std::unique_ptr<Body> handle_; // opaque pointer
// Note that a proper library component would use a raw pointer
// with hand-rolled memory management.
public:
  // Alias for the type of the body handled by this class.
  using body_type = Body;

  // Create a 'Handle' object having unique owenrship of a dynamically created
  // 'Body' object, using its default constructor.
  Handle();

  // Left out because slideware: generic forwarding constructors.

  /* Rule of 5 */
  Handle(Handle const& other);
  Handle(Handle && other) noexcept;

  ~Handle() noexcept;

  Handle& operator = (Handle const& other);
  Handle& operator = (Handle && other) noexcept;

  // Return a pointer to the body handled by this object.
  // The behaviour is undefined if this object is in a moved-from state.
  body_type* operator -> () const noexcept;
};

} // namespace lib

#endif // HANDLE_H_INCLUDE_GUARD
