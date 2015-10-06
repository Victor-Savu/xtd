#include <string>
#include <gtest/gtest.h>
#include <xtd/optional.hh>

TEST(optional, some) {
  const int i = 10;
  xtd::optional<int> maybe_int = xtd::some(i);
  xtd::optional<int> maybe_not = xtd::none{};
  xtd::optional<std::string> maybe_hello{"Goodbye!"};
  xtd::optional<xtd::optional<std::string>> not_event_that;
}

TEST(optional, equality) {
  EXPECT_EQ(xtd::some(10), xtd::some(10));
}

TEST(optional, match) {
  xtd::optional<int> surely_ten{10};
  xtd::optional<int> not_at_all;

  // no return type
  surely_ten.match([](int i) { EXPECT_EQ(i, 10); },
                   []() { ADD_FAILURE() << "Shouldn't have done that!"; });
  not_at_all.match([](int) { ADD_FAILURE() << "Shouldn't have done that!"; },
                   []() { SUCCEED() << "There really is nothing here."; });

  auto ten = surely_ten.match([](int i) { return i; }, []() { return 0; });
  EXPECT_EQ(ten, 10);
  auto zero = not_at_all.match([](int i) { return i; }, []() { return 0; });
  EXPECT_EQ(zero, 0);

  auto must_be_some =
      surely_ten.match([](int i) { return "Some"; }, []() { return "None"; });
  EXPECT_STREQ(must_be_some, "Some");
  auto must_be_none =
      not_at_all.match([](int i) { return "Some"; }, []() { return "None"; });
  EXPECT_STREQ(must_be_none, "None");

  using namespace std::literals::string_literals;
  auto just_hello = xtd::some("Hello"s);
  auto message = just_hello.match([](auto s) { return s + " world!"; },
                                  []() { return ""; });
  EXPECT_EQ(message, "Hello world!");
  just_hello = xtd::none{};
  message = just_hello.match([](auto s) { return s + " world!"; },
                             []() { return "We'll always have Paris."; });
  EXPECT_EQ(message, "We'll always have Paris.");
}

TEST(optional, map) {
  using namespace std::literals::string_literals;

  auto maybe_int = xtd::some(42);
  auto maybe_string = maybe_int.map([](int i) { return std::to_string(i); });
  maybe_string.match([](std::string const& s) { EXPECT_EQ(s, "42"s); },
                     []() { ADD_FAILURE() << "Shouldn't have done that!"; });

  xtd::optional<int> not_an_int = xtd::none{};
  auto not_a_string = not_an_int.map([](int i) { return std::to_string(i); });
  not_a_string.match([](std::string const&) { ADD_FAILURE() << "Shouldn't have done that!"; },
      []() { SUCCEED() << "none maps to none"; });

}
