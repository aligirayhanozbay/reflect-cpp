#ifndef RFL_REF_HPP_
#define RFL_REF_HPP_

#include <memory>
#include <stdexcept>

namespace rfl {

/// The Ref class behaves very similarly to the shared_ptr, but unlike the
/// shared_ptr, it is 100% guaranteed to be filled at all times.
template <class T>
class Ref {
 public:
  /// The default way of creating new references is
  /// Ref<T>::make(...) or make_ref<T>(...).
  template <class... Args>
  static Ref<T> make(Args&&... _args) {
    return Ref<T>(std::make_shared<T>(std::forward<Args>(_args)...));
  }

  Ref() : ptr_(std::make_shared<T>()) {}

  Ref(const Ref<T>& _other) = default;

  Ref(Ref<T>&& _other) = default;

  template <class U, typename std::enable_if<std::is_convertible_v<U, T>,
                                             bool>::type = true>
  Ref(const Ref<U>& _other) : ptr_(_other.ptr()) {}

  template <class U, typename std::enable_if<std::is_convertible_v<U, T>,
                                             bool>::type = true>
  Ref(Ref<U>&& _other) noexcept
      : ptr_(std::make_shared<T>(std::forward<U>(*_other))) {}

  ~Ref() = default;

  /// Returns a pointer to the underlying object
  T* get() const { return ptr_.get(); }

  /// Returns the underlying object.
  T& operator*() { return *ptr_; }

  /// Returns the underlying object.
  T& operator*() const { return *ptr_; }

  /// Returns the underlying object.
  T* operator->() { return ptr_.get(); }

  /// Returns the underlying object.
  T* operator->() const { return ptr_.get(); }

  /// Returns the underlying shared_ptr
  const std::shared_ptr<T>& ptr() const { return ptr_; }

  /// Copy assignment operator.
  template <class U, typename std::enable_if<std::is_convertible_v<U, T>,
                                             bool>::type = true>
  Ref<T>& operator=(const Ref<U>& _other) {
    ptr_ = _other.ptr();
    return *this;
  }

  /// Move assignment operator
  template <class U, typename std::enable_if<std::is_convertible_v<U, T>,
                                             bool>::type = true>
  Ref<T>& operator=(Ref<U>&& _other) noexcept {
    ptr_ = std::make_shared<T>(std::forward<U>(*_other));
    return *this;
  }

  /// Move assignment operator
  Ref<T>& operator=(Ref<T>&& _other) noexcept = default;

  /// Copy assignment operator
  Ref<T>& operator=(const Ref<T>& _other) = default;

 private:
  /// Only make is allowed to use this constructor.
  explicit Ref(std::shared_ptr<T>&& _ptr) : ptr_(std::move(_ptr)) {}

 private:
  /// The underlying shared_ptr_
  std::shared_ptr<T> ptr_;
};

/// Generates a new Ref<T>.
template <class T, class... Args>
Ref<T> make_ref(Args&&... _args) {
  return Ref<T>::make(std::forward<Args>(_args)...);
}

template <class T1, class T2>
inline auto operator<=>(const Ref<T1>& _t1, const Ref<T2>& _t2) {
  return _t1.ptr() <=> _t2.ptr();
}

template <class CharT, class Traits, class T>
inline std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& _os, const Ref<T>& _b) {
  _os << _b.get();
  return _os;
}

}  // namespace rfl

namespace std {

template <class T>
struct hash<rfl::Ref<T>> {
  size_t operator()(const rfl::Ref<T>& _r) const {
    return hash<shared_ptr<T>>()(_r.ptr());
  }
};

template <class T>
inline void swap(rfl::Ref<T>& _r1, rfl::Ref<T>& _r2) {
  return swap(_r1.ptr(), _r2.ptr());
}

}  // namespace std

#endif  // RFL_REF_HPP_

