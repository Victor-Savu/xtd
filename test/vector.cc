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

TEST(vector, back) {
  xtd::vector<int> v;
  v.push(10).push(12);
  v.back().match([](auto& v) { EXPECT_EQ(v, 12); },
                 []() { ADD_FAILURE() << "The vector should not be empty!"; });
  auto last = v.back().map([](auto& v) -> int { return v; });
  auto popped = v.pop().map([](auto& v) -> int { return v; });
  last.match(
      [&popped](auto& v) {
        popped.match([&v](auto& u) { int V{v}, U{u}; EXPECT_EQ(V, U); },
                     []() {
                       ADD_FAILURE()
                           << "There should be something that was popped.";
                     });
      },
      []() {
        return ADD_FAILURE() << "There should be something at the back.";
      });
}

TEST(vector, empty) {
  xtd::vector<int> v;
  EXPECT_EQ(v.empty(), true);
  v.push(10);
  EXPECT_EQ(v.empty(), false);
  v.pop();
  EXPECT_EQ(v.empty(), true);
}
