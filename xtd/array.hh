#pragma once

#include "emplacer.hh"

namespace xtd {

template <typename T, typename Memory>
struct array {
  Memory m_data;

  template<typename... Arg>
  explicit array(Arg&&... arg) : m_data{std::forward<Arg>(arg)...} {}

  template <typename IndexT>
  T& operator[](IndexT idx) {
    return *reinterpret_cast<T*>(&m_data[idx]);
  }

  template <typename IndexT>
  T const& operator[](IndexT idx) const {
    return *reinterpret_cast<T const*>(&m_data[idx]);
  }

  template <typename IndexT>
  emplacer<T> overwrite(IndexT idx) {
    return emplacer<T>{m_data[idx]};
  }
};

}
