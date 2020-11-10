#ifndef HANDLE_IMPL_H_INCLUDE_GUARD
#define HANDLE_IMPL_H_INCLUDE_GUARD
// HandleImpl.h

// Handle.h ///////////////////////////////////////////////////////////////////
#ifndef HANDLE_H_INCLUDE_GUARD
#define HANDLE_H_INCLUDE_GUARD

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

  // Return a pointer to the body handled by this object, enforcing const-correctness.
  // The behaviour is undefined if this object is in a moved-from state.
  body_type const* operator -> () const noexcept;
  body_type      * operator -> ()       noexcept;
};

} // namespace lib

#endif // HANDLE_H_INCLUDE_GUARD
///////////////////////////////////////////////////////////////////////////////
#include <cassert>
#include <utility> // move

template<class Body>
lib::Handle<Body>::Handle()
: handle_{std::make_unique<Body>()}
{ }
// NB: other forwarding ctors would be implemented similarly.

template<class Body>
lib::Handle<Body>::Handle(Handle const& other)
{
  // Correctly copy moved-from handles.
  if (other.handle_ != nullptr) {
    handle_ = std::make_unique<Body>(*other.handle_);
  }
}

template<class Body>
lib::Handle<Body>::Handle(Handle&& other) noexcept = default;

template<class Body>
lib::Handle<Body>::~Handle() noexcept = default;

template<class Body>
auto lib::Handle<Body>::operator = (Handle const& other) -> Handle&
{
  // A variation of the copy-and-swap idiom.
  auto tmp = other;
  *this = std::move(tmp);
  return *this;
}

template<class Body>
auto lib::Handle<Body>::operator = (Handle && other) noexcept
-> Handle& = default;

template<class Body>
auto lib::Handle<Body>::operator -> () const noexcept
-> body_type const*
{
  assert( handle_ != nullptr );
  return handle_.get();
}

template<class Body>
auto lib::Handle<Body>::operator -> () noexcept
-> body_type*
{
  assert( handle_ != nullptr );
  return handle_.get();
}

#endif // HANDLE_IMPL_H_INCLUDE_GUARD
