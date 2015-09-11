#pragma once
#include <memory>
#include <utility>
#include <vector>

#include "array.hh"
#include "optional.hh"

namespace {

static uint64_t algoLog2(uint64_t x) {
  uint64_t y;
  __asm__("\tbsr %1, %0\n" : "=r"(y) : "r"(x));
  return y;
}
};

namespace xtd {

template <typename T, uint8_t N = 0>
class vector {
  static constexpr size_t segmentCapacity() { return 1 << N; }
  static constexpr size_t segmentSize() {
    return sizeof(T) * segmentCapacity();
  }

  using segment_index_t = int;
  using index_t = int;
  using segment_t = array<T, std::aligned_storage_t<sizeof(T), alignof(T)>[segmentCapacity()]>;
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
  static auto& at(Vector& v, size_t p) {
    size_t elm = p & (v.segmentCapacity() - 1);
    size_t pos = p >> N;

    if (pos == 0) return v.m_data[0][0][elm];
    if (pos == 1) return v.m_data[1][0][elm];
    if (pos == 2) return v.m_data[1][1][elm];

    pos += 1;
    const uint64_t k = algoLog2(pos);  // the number of the superblock

    const uint64_t kdiv2 = k >> 1;
    const uint64_t oneShlKdiv2 = (1 << kdiv2);
    const uint64_t notKdiv2 = oneShlKdiv2 - 1;

    const uint64_t mask_b = (notKdiv2 << kdiv2)
                            << (k & 1);  // the first floor(k/2) bits after the
                                         // most significant bit in k
    const uint64_t mask_seg = (oneShlKdiv2 << (k & 1)) -
                              1;  // the least significant ceil(k/2) bits in k

    const uint64_t b =
        ((pos & mask_b) >> kdiv2) >>
        (k & 1);  // the index of the data block in the k-th superblock

    const uint64_t seg =
        pos & mask_seg;  // the index of the data segment in the b-th data block

    return v.m_data[(notKdiv2 << 1) + (k & 1) * oneShlKdiv2 + b][seg][elm];
  }

  T& operator[](size_t p) { return at(*this, p); }

  T const& operator[](size_t p) const { return at(*this, p); }
  /*
    template<typename Vector>
    friend auto back(Vector& v) {
      return (m_od) ? xtd::optional<>{ (oCast(char*, arr->end[arr->d-1]) -
    arr->element_size;
    }
  */
  optional<std::reference_wrapper<T>> back() {
    return (m_od) ? optional<std::reference_wrapper<
                        T>>{m_data[m_d - 1][m_od - 1][m_oseg - 1]}
                  : optional<std::reference_wrapper<T>>{xtd::none{}};
  }

  optional<std::reference_wrapper<T const>> back() const {
    return (m_od) ? optional<std::reference_wrapper<
                        T const>>{m_data[m_d - 1][m_od - 1][m_oseg - 1]}
                  : optional<std::reference_wrapper<T const>>{xtd::none{}};
  }

  bool empty() const { return size() == 0; }

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
};
}
