#include <iostream>
#include <string>

namespace xtd {
struct none {};

template <typename T>
class optional {
  bool some{false};
  std::aligned_storage_t<sizeof(T), alignof(T)> val;

 public:
  optional() : some{false} {}
  explicit optional(T t) : some{true} { new (&val) T(std::move(t)); }

  optional(optional const& opt) : some{opt.some} {
    if (some) {
      val = opt.val;
    }
  }

  optional& operator=(optional opt) noexcept { swap(*this, opt); }

  friend void swap(optional& a, optional& b) noexcept {
    if (a.some || b.some) {
      using std::swap;
      swap(a.val, b.val);
      swap(a.some, b.some);
    }
  }

  optional(none) {}

  ~optional() {
    if (some) {
      reinterpret_cast<T*>(&val)->~T();
    }
  }

  template <typename OnSome, typename OnNone>
  void match(OnSome&& on_some, OnNone&& on_none) const {
    (some) ? on_some(*reinterpret_cast<T const*>(&val)) : on_none();
  }

  template <typename OnSome, typename OnNone>
  auto match(OnSome&& on_some, OnNone&& on_none) const {
    return (some) ? on_some(*reinterpret_cast<T const*>(&val)) : on_none();
  }

  template <typename OnSome, typename OnNone>
  auto match(OnSome&& on_some, OnNone&& on_none) {
    return (some) ? on_some(*reinterpret_cast<T*>(&val)) : on_none();
  }

  template <typename Map>
  auto map(Map&& mapper) {
    using Ret = typename std::result_of<Map(T)>::type;
    return (some) ? optional<Ret>{mapper(*reinterpret_cast<T*>(&val))} : optional<Ret>{none{}};
  }
};

template <typename T>
auto some(T&& t) -> optional<std::remove_cv_t<std::remove_reference_t<std::remove_cv_t<T>>>> {
  return optional<std::remove_cv_t<std::remove_reference_t<std::remove_cv_t<T>>>>(std::forward<T&&>(t));
}
}
