#include <iostream>
#include <string>
#include <type_traits>

namespace xtd {
struct none {};

template <typename T>
class optional {
  bool some{false};
  std::aligned_storage_t<sizeof(T), alignof(T)> val;

  T& asT() & { return *reinterpret_cast<T*>(&val); }
  T const& asT() const& { return *reinterpret_cast<T const*>(&val); }
  T asT() && { return {std::move(*reinterpret_cast<T*>(&val))}; }

  template <typename Optional, typename OnSome, typename OnNone>
  static auto match(Optional&& opt, OnSome&& on_some, OnNone&& on_none) {
    return (opt.some) ? std::forward<OnSome>(on_some)(
                            std::forward<Optional>(opt).asT())
                      : std::forward<OnNone>(on_none)();
  }

  template <typename Optional, typename Map>
  static auto map(Optional&& opt, Map&& m) {
    using Ret =
        decltype(std::forward<Map>(m)(std::forward<Optional>(opt).asT()));
    return (opt.some) ? optional<Ret>{std::forward<Map>(m)(
                            std::forward<Optional>(opt).asT())}
                      : optional<Ret>{none{}};
  }

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
      } else if (some) {
        asT().~T();
      }
      some = other.some;
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
  auto match(OnSome&& on_some, OnNone&& on_none) const& {
    return match(*this, std::forward<OnSome>(on_some),
                 std::forward<OnNone>(on_none));
  }

  template <typename OnSome, typename OnNone>
  auto match(OnSome&& on_some, OnNone&& on_none) & {
    return match(*this, std::forward<OnSome>(on_some),
                 std::forward<OnNone>(on_none));
  }

  template <typename OnSome, typename OnNone>
  auto match(OnSome&& on_some, OnNone&& on_none) && {
    return match(std::move(*this), std::forward<OnSome>(on_some),
                 std::forward<OnNone>(on_none));
  }

  template <typename Map>
  auto map(Map&& m) const& {
    return map(*this, std::forward<Map>(m));
  }

  template <typename Map>
  auto map(Map&& m) & {
    return map(*this, std::forward<Map>(m));
  }

  template <typename Map>
  auto map(Map&& m) && {
    return map(std::move(*this), std::forward<Map>(m));
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
