#include <iostream>
#include <string>
#include <type_traits>

namespace xtd {
struct none {};

template <typename T>
class optional {
  bool some{false};
  std::aligned_storage_t<sizeof(T), alignof(T)> val;

  T& asT() { return *reinterpret_cast<T*>(&val); }
  T const& asT() const { return *reinterpret_cast<T const*>(&val); }

 public:
  optional() = default;

  template <typename Head, typename... Args>
  explicit optional(Head&& h, Args&&... t)
      : some{true} {
    new (&val) T{std::forward<Head>(h), std::forward<Args>(t)...};
  }

  optional(optional&& other) : some(other.some) {
    if (some) {
      static_assert(std::is_same<T&&, decltype(std::move(other.asT()))>::value,
                    "Buba");
      new (&val) T{std::move(other.asT())};
    }
    other.some = false;
  }

  optional(optional const& other) : some(other.some) {
    if (some) {
      new (&val) T{other.asT()};
    }
  }

  optional(optional& other) : optional{static_cast<optional const&>(other)} {}

  optional& operator=(optional const& other) {
    if (this != &other) {
      if (other.some)
        if (some)
          asT() = other.asT();
        else
          new (&val) T{other.asT()};
      else if (some)
        asT().~T();
      some = other.some;
    }
    return *this;
  }

  optional& operator=(optional&& other) {
    if (this != &other) {
      if (other.some) {
        if (some)
          asT() = std::move(other.asT());
        else
          new (&val) T{std::move(other.asT())};
        other.asT().~T();
      } else if (some) {
        asT().~T();
      }

      some = other.some;
      other.some = false;
    }
    return *this;
  }

  friend void swap(optional& a, optional& b) noexcept {
    if (&a != &b) {
      using std::swap;
      swap(a.val, b.val);
      swap(a.some, b.some);
    }
  }

  optional(none) {}

  ~optional() {
    if (some) {
      asT().~T();
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
    return (some) ? on_some(asT()) : on_none();
  }

  template <typename Map>
  auto map(Map mapper) const {
    using Ret = decltype(mapper(asT()));
    return (some) ? optional<Ret>{mapper(asT())} : optional<Ret>{none{}};
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
