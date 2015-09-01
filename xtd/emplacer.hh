#pragma once

namespace xtd {

template <typename T>
class emplacer {
  void* place;

 public:
  template <typename U>
  explicit emplacer(U& u)
      : place{&u} {
    static_assert(sizeof(U) == sizeof(T),
                  "Cannot initialize emplacer<T> with a reference to a type of "
                  "a different size than sizeof(T).");
  }

  template <typename... Args>
  void emplace(Args&&... args) const {
    new (place) T(std::forward<Args>(args)...);
  }
};
}
