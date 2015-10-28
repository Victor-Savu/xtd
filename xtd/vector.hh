#pragma once
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "array.hh"
#include "optional.hh"

namespace xtd {

template<typename T, typename U>
auto opt_ref(U&, T& t) { return std::ref(t); };

template<typename T, typename U>
auto opt_ref(U const&, T const& t) { return std::ref(t); };

template<typename T, typename U>
auto opt_ref(U&&, T&& t) { return std::move(t); };

template <typename T, uint8_t N = 0>
class vector {
  static constexpr size_t segmentCapacity() { return 1 << N; }
  static constexpr size_t segmentSize() {
    return sizeof(T) * segmentCapacity();
  }

  using segment_index_t = int;
  using index_t = int;
  using segment_t =
      array<T,
            std::aligned_storage_t<sizeof(T), alignof(T)>[segmentCapacity()]>;
  using block_t = array<segment_t, std::unique_ptr<segment_t[]>>;

  std::vector<block_t> m_data;  // begin, end
  uint32_t m_d;

  uint32_t m_s;
  uint32_t m_n;
  uint32_t m_od;
  uint32_t m_nd;
  uint32_t m_os;
  uint32_t m_ns;
  bool m_emptyDb;

  uint32_t m_oseg;

  void grow() {
    if (m_od == m_nd) {
      if (m_os == m_ns) {
        if (++m_s & 1)
          m_ns <<= 1;
        else
          m_nd <<= 1;
        m_os = 0;
      }
      if (!m_emptyDb) {
        m_data.emplace_back(new segment_t[m_nd]);
      }
      m_emptyDb = 0;
      ++m_d;
      ++m_os;
      m_od = 0;
    }
    ++m_n;
    ++m_od;
  }

  void shrink() {
    --m_n;
    --m_od;
    if (m_od == 0) {
      if (m_emptyDb) m_data.pop_back();
      --m_d;
      --m_os;
      if (m_os == 0) {
        --m_s;
        if (!(m_s & 1))
          m_ns >>= 1;
        else
          m_nd >>= 1;
        m_os = m_ns;
      }
      m_od = m_nd;
      m_emptyDb = true;
    }
    if (m_n == 0) {
      m_od = 1;
      m_os = 0;
      m_nd = 1;
      m_ns = 1;
      m_s = 1;
      m_d = 0;
      m_oseg = segmentCapacity();
      m_emptyDb = true;
    }
  };

  template <typename Vector>
  static auto& unsafe_at(Vector& v, size_t p) {
    size_t elm = p & (v.segmentCapacity() - 1);
    size_t pos = p >> N;

    if (pos == 0) return v.m_data[0][0][elm];
    if (pos == 1) return v.m_data[1][0][elm];
    if (pos == 2) return v.m_data[1][1][elm];

    pos += 1;
    // the number of the superblock
    uint64_t k;
    __asm__("\tbsr %1, %0\n" : "=r"(k) : "r"(pos));

    const uint64_t kdiv2 = k >> 1;
    const uint64_t oneShlKdiv2 = (1 << kdiv2);
    const uint64_t notKdiv2 = oneShlKdiv2 - 1;

    // the first floor(k/2) bits after the most significant bit in k
    const uint64_t mask_b = (notKdiv2 << kdiv2) << (k & 1);

    // the least significant ceil(k/2) bits in k
    const uint64_t mask_seg = (oneShlKdiv2 << (k & 1)) - 1;

    // the index of the data block in the k-th superblock
    const uint64_t b = ((pos & mask_b) >> kdiv2) >> (k & 1);

    // the index of the data segment in the b-th data block
    const uint64_t seg = pos & mask_seg;

    return v.m_data[(notKdiv2 << 1) + (k & 1) * oneShlKdiv2 + b][seg][elm];
  };

 public:
  vector()
      : m_d{0},
        m_s{1},
        m_n{0},
        m_oseg{segmentCapacity()},
        m_od{1},
        m_os{0},
        m_nd{1},
        m_ns{1},
        m_emptyDb{true} {
    m_data.emplace_back(new segment_t[m_nd]);
  }

  template <typename... Args>
  vector& push(Args&&... args) {
    if (m_oseg < segmentCapacity())
      m_oseg++;
    else {
      grow();
      m_oseg = 1;
    }
    m_data[m_d - 1][m_od - 1]
        .overwrite(m_oseg - 1)
        .emplace(std::forward<Args>(args)...);
    return *this;
  }

  template <typename Vector>
  static auto at(Vector&& v, size_t p) {
    return xtd::opt(p < v.size(),
                  xtd::opt_ref(std::forward<Vector>(v), unsafe_at(v, p)));
  }

  auto operator[](size_t p) & { return at(*this, p); }

  auto operator[](size_t p) const & { return at(*this, p); }

  auto operator[](size_t p) && { return at(std::move(*this), p); }

  template <typename Vector>
  static auto back(Vector& v) {
    return xtd::opt(v.m_od,
                    std::ref(v.m_data[v.m_d - 1][v.m_od - 1][v.m_oseg - 1]));
  }

  auto back() { return back(*this); }

  auto back() const { return back(*this); }

  bool empty() const { return m_n == 0; }

  size_t size() const { return (m_n) ? (((m_n - 1) << N) + m_oseg) : (0); }

  optional<T> pop() {
    auto ret = back().map([](T& t) -> T {
      T tc{std::move(t)};
      t.~T();
      return tc;
    });

    if (!--m_oseg) {
      shrink();
      m_oseg = segmentCapacity();
    }

    return ret;
  }

  constexpr T getDefaultValue() { return {}; }
  constexpr T const getDefaultValue() const { return {}; }

  template <typename Container>
  class iterator_t
      : public std::iterator<std::random_access_iterator_tag,
                             decltype(Container().getDefaultValue())> {
    using super_t = std::iterator<std::random_access_iterator_tag,
                                  decltype(Container().getDefaultValue())>;
    using iter_mut = vector::template iterator_t<vector>;
    friend class vector::template iterator_t<vector const>;

    size_t i{0};
    Container* v{0};

   public:
    iterator_t() = default;
    iterator_t(Container& v, size_t i) : v{&v}, i{i} {}

    // Any iterator (over either const or mutable) can be instantiated from an
    // iterator over mutables.
    iterator_t(iter_mut const& other) : v{other.v}, i{other.i} {}

    // EqualityComparable
    bool operator==(iterator_t const& it) const {
      return (i == it.i) && (v == it.v);
    }

    // InputIterator && ForwardIterator
    bool operator!=(iterator_t const& it) const { return !(*this == it); }
    auto& operator++() {
      i++;
      return *this;
    }
    typename super_t::reference operator*() const { return unsafe_at(*v, i); }

    typename super_t::reference operator->() const {
      return Container::unsafe_at(*v, i);
    }

    auto operator++(int) {
      iterator_t it{*this};
      ++(*this);
      return it;
    }

    // bidirectional iterator
    auto& operator--() {
      --i;
      return *this;
    }
    auto operator--(int) {
      iterator_t it{*this};
      --(*this);
      return it;
    }

    // random access iterator
    auto& operator+=(typename super_t::difference_type n) {
      i += n;
      return *this;
    }

    auto operator+(typename super_t::difference_type n) const {
      iterator_t copy{*this};
      copy += n;
      return copy;
    }

    auto& operator-=(typename super_t::difference_type n) {
      i -= n;
      return *this;
    }

    auto operator-(typename super_t::difference_type n) const {
      iterator_t copy{*this};
      copy -= n;
      return copy;
    }

    typename super_t::difference_type operator-(iterator_t it) const {
      return i - it.i;
    }

    auto& operator[](typename super_t::difference_type n) const {
      return v->at(i + n);
    }

    bool operator<(iterator_t it) const { return (*this) - it < 0; }

    bool operator>(iterator_t it) const { return (*this) - it > 0; }

    bool operator<=(iterator_t it) const { return (*this) - it <= 0; }

    bool operator>=(iterator_t it) const { return (*this) - it >= 0; }

    // output iterator
  };

  using iterator = iterator_t<vector>;
  using const_iterator = iterator_t<vector const>;

  iterator begin() { return {*this, 0}; }
  iterator end() { return {*this, size()}; }

  const_iterator begin() const { return {*this, 0}; }
  const_iterator end() const { return {*this, size()}; }

  const_iterator cbegin() const { return {*this, 0}; }
  const_iterator cend() const { return {*this, size()}; }
};
}

template <typename T>
auto operator+(
    typename std::vector<std::remove_cv_t<T>>::template iterator_t<
        T>::difference_type n,
    typename std::vector<std::remove_cv<T>>::template iterator_t<T> it) {
  return it + n;
}
