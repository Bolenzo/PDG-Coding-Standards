#ifndef HANDLE_IMPL_H_INCLUDE_GUARD
#define HANDLE_IMPL_H_INCLUDE_GUARD
// HandleImpl.h

// Handle.h ///////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////
#include <cassert>
#include <memory>   // unique_ptr
#include <new>      // launder
#include <utility>  // move

namespace {

template<class Body, std::size_t Size>
bool constexpr fits  = sizeof(Body) <= Size
                    && alignof(Body) <= alignof(std::aligned_storage_t<Size>);

// Syntactic sugar that returns a reference to a 'Body' from a 'Handle'.
template<class Body, std::size_t Size>
auto body(lib::Handle<Body, Size> const& handle) noexcept
-> typename lib::Handle<Body, Size>::body_type const&
{
  return *(handle.operator->());
}
template<class Body, std::size_t Size>
auto body(lib::Handle<Body, Size>& handle) noexcept
-> typename lib::Handle<Body, Size>::body_type&
{
  return *(handle.operator->());
}

} // unnamed namespace

template<class Body, std::size_t Size>
lib::Handle<Body, Size>::Handle()
{
  if constexpr (::fits<Body, Size>) { // allocate in-place
    ::new (&storage_) Body();
  }
  else { // allocate dynamically
    using BodyPtr = std::unique_ptr<Body>;
    static_assert( ::fits<BodyPtr, Size>, "storage cannot hold a pointer");
    ::new (&storage_) BodyPtr(std::make_unique<Body>());
  }
}
// NB: other forwarding ctors would be implemented similarly.

template<class Body, std::size_t Size>
lib::Handle<Body, Size>::Handle(Handle const& other)
{
  if constexpr (::fits<Body, Size>) { // allocate in-place
    ::new (&storage_) Body(::body(other));
  }
  else { // allocate dynamically
    using BodyPtr = std::unique_ptr<Body>;
    static_assert( ::fits<BodyPtr, Size>, "storage cannot hold a pointer");
    // Correctly copy moved-from handles...
    auto const& src_p = *std::launder(reinterpret_cast<BodyPtr*>(&other.storage_));
    if (src_p != nullptr) { // clone
      ::new (&storage_) BodyPtr(std::make_unique<Body>(::body(other)));
    } 
    else { // construct null unique_ptr, i.e. moved-from large body.
      ::new (&storage_) BodyPtr(nullptr);
    }
  }
}

template<class Body, std::size_t Size>
lib::Handle<Body, Size>::Handle(Handle&& other) noexcept
{
  if constexpr (::fits<Body, Size>) { // move-construct in-place
    ::new (&storage_) Body(std::move(::body(other)));
  }
  else { // move unique_ptr
    using BodyPtr = std::unique_ptr<Body>;
    static_assert( ::fits<BodyPtr, Size>, "storage cannot hold a pointer");
    auto& src_p = *std::launder(reinterpret_cast<BodyPtr*>(&other.storage_));
    ::new (&storage_) BodyPtr(std::move(src_p));
  }
}

template<class Body, std::size_t Size>
lib::Handle<Body, Size>::~Handle() noexcept
{
  if constexpr (::fits<Body, Size>) { // destroy body
    ::body(*this).~body_type();
  }
  else { // destroy unique_ptr
    using BodyPtr = std::unique_ptr<Body>;
    auto& p = *std::launder(reinterpret_cast<BodyPtr*>(&storage_));
    p.reset(); // equivalent of calling destructor
  }
}

template<class Body, std::size_t Size>
auto lib::Handle<Body, Size>::operator = (Handle const& other) -> Handle&
{
  // A variation of the copy-and-swap idiom.
  auto tmp = other;
  *this = std::move(tmp);
  return *this;
}

template<class Body, std::size_t Size>
auto lib::Handle<Body, Size>::operator = (Handle && other) noexcept
-> Handle&
{
  if constexpr (::fits<Body, Size>) { // move-assign in-place
    ::body(*this) = std::move(::body(other));
  }
  else { // move-assign unique_ptr
    using BodyPtr = std::unique_ptr<Body>;
    static_assert( ::fits<BodyPtr, Size>, "storage cannot hold a pointer");
    auto& src_p = *std::launder(reinterpret_cast<BodyPtr*>(&other.storage_));
    auto& dst_p = *std::launder(reinterpret_cast<BodyPtr*>(&storage_));
    dst_p = std::move(src_p);
  }
  return *this;
}

template<class Body, std::size_t Size>
auto lib::Handle<Body, Size>::operator -> () const noexcept
-> body_type const*
{
  if constexpr (::fits<Body, Size>) { // allocated in-place
    return std::launder(reinterpret_cast<body_type const*>(&storage_));
  }
  else { // allocated dynamically
    auto const& result = *std::launder(reinterpret_cast<std::unique_ptr<Body>*>(&storage_));
    assert( result != nullptr );
    return result.get();
  }
}

template<class Body, std::size_t Size>
auto lib::Handle<Body, Size>::operator -> () noexcept
-> body_type*
{
  // The somewhat infamous "const_cast overload" idiom.
  auto const const_ptr = static_cast<Handle const*>(this)-> operator ->();
  return const_cast<body_type*>(const_ptr);
}

#endif // HANDLE_IMPL_H_INCLUDE_GUARD
