#include <xtd/vector.hh>

#include <string>
#include <algorithm>

#include <gtest/gtest.h>

TEST(vector_iterator, construction) {
  xtd::vector<int> v;
  xtd::vector<int> const cv;

  xtd::vector<int>::iterator def_mut;
  xtd::vector<int>::iterator mut{v, 0};

  xtd::vector<int>::iterator mut_copy{mut};

  xtd::vector<int>::const_iterator def_ct;
  xtd::vector<int>::const_iterator ct{cv, 0};

  xtd::vector<int>::const_iterator ct_copy{ct};

  xtd::vector<int>::const_iterator ct_copy_from_mut{mut};
}

TEST(vector_iterator, range_for) {
  xtd::vector<std::string> v;
  std::string msg;

  v.push("Hello");
  v.push(" world!");
  for (auto&& s : v) msg += s;
  EXPECT_EQ("Hello world!", msg);
}

TEST(vector_iterator, accumulate) {
  xtd::vector<std::string> v;
  v.push("Hello");
  v.push(" world!");

  std::string msg = std::accumulate(std::begin(v), std::end(v), std::string{});
  EXPECT_EQ("Hello world!", msg);

  xtd::vector<std::string>& w{v};
  std::string msgw = std::accumulate(std::begin(w), std::end(w), std::string{});
  EXPECT_EQ("Hello world!", msgw);
}

TEST(vector_iterator, deref) {
  xtd::vector<std::string> v;
  v.push("Hello");
  v.push(" world!");

  auto i = v.begin();
  EXPECT_EQ("Hello", *i);
}

TEST(vector_iterator, pre_increment) {
  xtd::vector<std::string> v;
  v.push("Hello");
  v.push(" world!");

  auto i = v.begin();
  EXPECT_EQ(" world!", *(++i));
  EXPECT_EQ(" world!", *i);
}

TEST(vector_iterator, post_increment) {
  xtd::vector<std::string> v;
  v.push("Hello");
  v.push(" world!");

  auto i = v.begin();
  EXPECT_EQ("Hello", *(i++));
  EXPECT_EQ(" world!", *i);
}

TEST(vector_iterator, equality_comparable) {
  xtd::vector<std::string> v;
  v.push("Hello");
  v.push(" world!");

  auto i = v.begin();
  auto j = i;
  EXPECT_EQ(i, j);
  EXPECT_NE(i, v.end());
}

