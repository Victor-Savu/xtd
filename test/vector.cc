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
  EXPECT_EQ(v[0], xtd::some(10));
  EXPECT_EQ(v[1], xtd::some(20));
  EXPECT_EQ(v[2], xtd::some(30));
}

TEST(vector, pop) {
  xtd::vector<int> v;
  v.push(10).pop().match(
      [](auto const& v) { EXPECT_EQ(v, 10); },
      []() { ADD_FAILURE() << "The vector should not be empty!"; });
}

TEST(vector, back) {
  xtd::vector<int> v;
  v.push(10).push(12);
  v.back().match([](auto const& v) { EXPECT_EQ(v, 12); },
                 []() { ADD_FAILURE() << "The vector should not be empty!"; });
  auto last = v.back().map([](auto v) -> int { return v; });
  auto popped = v.pop().map([](auto v) -> int { return v; });
  last.match(
      [&popped](auto const& v) {
        popped.match([&v](auto const& u) { EXPECT_EQ(v, u); },
                     []() {
                       ADD_FAILURE()
                           << "There should be something that was popped.";
                     });
      },
      []() {
        return ADD_FAILURE() << "There should be something at the back.";
      });

  auto const& cv = v;
  last = v.back().map([](auto v) -> int { return v; });
  auto last_cv = cv.back().map([](auto v) -> int { return v; });
  last.match(
      [&last_cv](auto const& v) {
        last_cv.match([&v](auto const& u) { EXPECT_EQ(v, u); },
                     []() {
                       ADD_FAILURE()
                           << "There should be something at the const back.";
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
