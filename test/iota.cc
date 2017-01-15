#include <gtest/gtest.h>
#include <xtd/iota.hh>

TEST(iota, end) {
  EXPECT_NE(xtd::iota(4), xtd::end);
}

TEST(iota, increment) {
  auto iota_4 = ++xtd::iota(3);
  EXPECT_EQ(xtd::iota(4), iota_4);
}

TEST(iota, dereference) {
  EXPECT_EQ(*xtd::iota(5), 5);
}

TEST(iota, for) {
  int n = 10;
  for (auto i=iota(10); i != xtd::end; ++i) {
    EXPECT_EQ(*i, n);
    ++n;
  }
}
