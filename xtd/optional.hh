#include <iostream>
#include <string>
#include <type_traits>

namespace xtd {
struct none {};

template <typename T>
class optional {
  bool some{false};
  std::aligned_storage_t<sizeof(T), alignof(T)> val;

 public:
  optional() : some{false} {}

  template <typename... Args>
  explicit optional(Args&&... t)
      : some{true} {
    new (&val) T(std::forward<Args>(t)...);
  }

  optional(optional const& opt) : some{opt.some} {
    if (some) {
      val = opt.val;
    }
  }

  optional& operator=(optional opt) noexcept {
    swap(*this, opt);
    return *this;
  }

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

  template <typename U>
  bool operator==(optional<U> const& other) const {
    return match(
        [&other, this](T const& t) {
          return other.match([this, &t](U const& u) { return u == t; },
                             []() { return false; });
        },
        [&other]() {
          return other.match([](U const&) { return false; },
                             []() { return true; });
        });
  }

  template <typename OnSome, typename OnNone>
  auto match(OnSome on_some, OnNone on_none) const {
    return (some) ? on_some(*reinterpret_cast<T const*>(&val)) : on_none();
  }

  template <typename Map>
  auto map(Map mapper) const {
    using Ret = decltype(mapper(*reinterpret_cast<T const*>(&val)));
    return (some) ? optional<Ret>{mapper(*reinterpret_cast<T const*>(&val))}
                  : optional<Ret>{none{}};
  }
};

template <typename T>
constexpr auto some(T&& t) {
  using ds_t = typename std::decay<T>::type;
  return optional<ds_t>(std::forward<T>(t));
}

template <typename T>
constexpr auto opt(bool cond, T&& t) {
  using ds_t = typename std::decay<T>::type;
  return cond ? optional<ds_t>(std::forward<T>(t)) : optional<ds_t>(none{});
}
}
