#pragma once

namespace xtd {

struct end {};

// Iterator
template<typename Num>
class Iota {
  Num n;

  template<typename Head, typename ...Tail>
  explicit constexpr Iota(Head&& head, Tail&& tail...)
      : n {std::forward<Head>(head), std::forward<Tail>(tail)...}
  {}

  constexpr auto next() & {
  }

  constexpr auto operator*() const& {
    return num;
  }

  constexpr auto operator*() & {
    return num;
  }

  constexpr auto operator*() && {
    return std::move(num);
  }

  constexpr auto operator++() & {
    ++n;
    return *this;
  }

  constexpr auto begin() {
    return *this;
  }

  constexpr auto end() {
    return end{};
  }
}

};
