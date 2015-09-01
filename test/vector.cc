#include <xtd/vector.hh>

#include <gtest/gtest.h>

TEST(vector, constructor) { xtd::vector<int> v; }

TEST(vector, push) {
  xtd::vector<int> v;
  v.push(1);
  v.push(10).push(20).push(30);
}

TEST(vector, at) {
  xtd::vector<int> v;
  v.push(10).push(20).push(30);
  EXPECT_EQ(v[0], 10);
  EXPECT_EQ(v[1], 20);
  EXPECT_EQ(v[2], 30);
}

TEST(vector, pop) {
  xtd::vector<int> v;
  v.push(10).pop().match(
      [](auto& v) { EXPECT_EQ(v, 10); },
      []() { ADD_FAILURE() << "The vector should not be empty!"; });
}

TEST(vector, empty) {
  xtd::vector<int> v;
  EXPECT_EQ(true, v.empty());
  v.push(10);
  EXPECT_EQ(false, v.empty());
  v.pop();
  EXPECT_EQ(true, v.empty());
}
