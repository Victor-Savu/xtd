#include <string>
#include <gtest/gtest.h>
#include <xtd/optional.hh>
#include <xtd/call_tracker.hh>

TEST(optional, some) {
  const int i = 10;
  xtd::optional<int> maybe_int = xtd::some(i);
  xtd::optional<int> maybe_not = xtd::none{};
  xtd::optional<std::string> maybe_hello{"Goodbye!"};
  xtd::optional<xtd::optional<std::string>> not_event_that;
}

TEST(optional, equality) { EXPECT_EQ(xtd::some(10), xtd::some(10)); }

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

struct CallCounter {
  CallCounter(int& rv_calls) : rv_calls{rv_calls} {}

  template <typename... T>
  void operator()(T&&...) & {
    ++calls;
  };

  template <typename... T>
  void operator()(T&&...) const & {
    ++const_calls;
  };

  template <typename... T>
  void operator()(T&&...) && {
    ++rv_calls;
  };

  int calls{0};
  mutable int const_calls{0};
  int& rv_calls;
};

TEST(optional, capturing_match) {
  int countSome{0};
  int countNone{0};

  CallCounter onSome{countSome};
  CallCounter onNone{countNone};

  xtd::opt(false, 3).match(onSome, onNone);
  EXPECT_EQ(0, onSome.calls);
  EXPECT_EQ(1, onNone.calls);
  EXPECT_EQ(0, onSome.const_calls);
  EXPECT_EQ(0, onNone.const_calls);
  EXPECT_EQ(0, countSome);
  EXPECT_EQ(0, countNone);

  xtd::some(3).match(onSome, onNone);
  EXPECT_EQ(1, onSome.calls);
  EXPECT_EQ(1, onNone.calls);
  EXPECT_EQ(0, onSome.const_calls);
  EXPECT_EQ(0, onNone.const_calls);
  EXPECT_EQ(0, countSome);
  EXPECT_EQ(0, countNone);

  CallCounter const onSomeConst{countSome};
  CallCounter const onNoneConst{countNone};

  xtd::opt(false, 3).match(onSomeConst, onNoneConst);
  EXPECT_EQ(0, onSomeConst.calls);
  EXPECT_EQ(0, onNoneConst.calls);
  EXPECT_EQ(0, onSomeConst.const_calls);
  EXPECT_EQ(1, onNoneConst.const_calls);
  EXPECT_EQ(0, countSome);
  EXPECT_EQ(0, countNone);

  xtd::some(3).match(onSomeConst, onNoneConst);
  EXPECT_EQ(0, onSomeConst.calls);
  EXPECT_EQ(0, onNoneConst.calls);
  EXPECT_EQ(1, onSomeConst.const_calls);
  EXPECT_EQ(1, onNoneConst.const_calls);
  EXPECT_EQ(0, countSome);
  EXPECT_EQ(0, countNone);

  xtd::opt(false, 3).match(CallCounter{countSome}, CallCounter{countNone});
  EXPECT_EQ(0, countSome);
  EXPECT_EQ(1, countNone);

  xtd::some(3).match(CallCounter{countSome}, CallCounter{countNone});
  EXPECT_EQ(1, countSome);
  EXPECT_EQ(1, countNone);
}

TEST(optional, returning_match) {
  std::vector<int> v{0, 1, 2, 3, 4, 5};

  // return by &
  ++xtd::some(2).match([&](auto&& idx) -> auto& { return v[idx]; },
                       [&]() -> auto& { return v.front(); });
  ++xtd::opt(false, 0).match([&](auto&& idx) -> auto& { return v[idx]; },
                             [&]() -> auto& { return v.front(); });
  EXPECT_EQ(1, v[0]);
  EXPECT_EQ(1, v[1]);
  EXPECT_EQ(3, v[2]);
  EXPECT_EQ(3, v[3]);
  EXPECT_EQ(4, v[4]);
  EXPECT_EQ(5, v[5]);

  // return by const&
  auto const& copy = xtd::some(4).match([&](auto idx) { return v[idx]; },
                                        [&]() { return v.front(); });
  auto& ref = xtd::some(4).match([&](auto idx) -> auto const& {
    return v[idx];
  }, [&]() -> auto const& { return v.front(); });

  ++v[4];

  EXPECT_EQ(5, v[4]);
  EXPECT_EQ(4, copy);
  EXPECT_EQ(5, ref);
}

TEST(optional, mutating_match) {
  auto about_ten = xtd::some(10);
  about_ten.match([](auto& val) { ++val; }, []() {});
  about_ten.match(
      [](auto const& val) { EXPECT_EQ(11, val); },
      []() { ADD_FAILURE() << "Match should not change availability."; });
}

TEST(optional, map) {
  using namespace std::literals::string_literals;

  auto maybe_int = xtd::some(42);
  auto maybe_string = maybe_int.map([](int i) { return std::to_string(i); });
  maybe_string.match([](std::string const& s) { EXPECT_EQ(s, "42"s); },
                     []() { ADD_FAILURE() << "Shouldn't have done that!"; });

  xtd::optional<int> not_an_int = xtd::none{};
  auto not_a_string = not_an_int.map([](int i) { return std::to_string(i); });
  not_a_string.match(
      [](std::string const&) { ADD_FAILURE() << "Shouldn't have done that!"; },
      []() { SUCCEED() << "none maps to none"; });
}

TEST(optional, move_construction) {
  int move_constructed_a{0};
  int copy_constructed_a{0};
  int move_constructed_b{0};
  int copy_constructed_b{0};
  int moved_from_destroyed{0};
  int destroyed{0};

  {
    xtd::call_tracker cta;
    cta.onMovedFromDestruction([&]() { ++moved_from_destroyed; });
    cta.onDestruction([&]() { ++destroyed; });

    xtd::call_tracker::onMoveConstruction([&]() { ++move_constructed_a; });
    xtd::call_tracker::onCopyConstruction([&]() { ++copy_constructed_a; });
    auto a = xtd::some(std::move(cta));

    xtd::call_tracker::onMoveConstruction([&]() { ++move_constructed_b; });
    xtd::call_tracker::onCopyConstruction([&]() { ++copy_constructed_b; });
    xtd::optional<xtd::call_tracker> b{std::move(a)};

    xtd::call_tracker::onMoveConstruction([]() {});
    xtd::call_tracker::onCopyConstruction([]() {});
  }

  EXPECT_EQ(1, move_constructed_a);
  EXPECT_EQ(1, move_constructed_b);
  EXPECT_EQ(0, copy_constructed_a);
  EXPECT_EQ(0, copy_constructed_b);
  EXPECT_EQ(1, moved_from_destroyed);
  EXPECT_EQ(1, destroyed);
}

TEST(optional, move) {
  int moved_into{0};
  int moved_to{0};

  int moved_into_new{0};
  int move_constructed{0};

  int moved_from_destroyed{0};
  int destroyed_a{0};
  int destroyed_b{0};

  xtd::call_tracker::onMoveConstruction([&]() { ++move_constructed; });

  {
    xtd::call_tracker cta, ctb;

    cta.onMovingInto([&]() { ++moved_into; });
    cta.onMovingIntoNew([&]() { ++moved_into_new; });
    cta.onMovedFromDestruction([&]() { ++moved_from_destroyed; });
    cta.onDestruction([&]() { ++destroyed_a; });

    ctb.onMoveAssign([&]() { ++moved_to; });
    ctb.onDestruction([&]() { ++destroyed_b; });

    auto a = xtd::some(cta);
    auto b = xtd::some(ctb);

    b = std::move(a);
  }

  xtd::call_tracker::onMoveConstruction([]() {});

  EXPECT_EQ(1, moved_into);
  EXPECT_EQ(1, moved_to);

  EXPECT_EQ(0, moved_into_new);
  EXPECT_EQ(0, move_constructed);

  EXPECT_EQ(1, moved_from_destroyed);
  EXPECT_EQ(2, destroyed_a);
  EXPECT_EQ(1, destroyed_b);
}

TEST(optional, copy_construction) {
  int move_constructed_a{0};
  int copy_constructed_a{0};
  int move_constructed_b{0};
  int copy_constructed_b{0};
  int moved_from_destroyed{0};
  int destroyed{0};

  {
    xtd::call_tracker cta;
    cta.onMovedFromDestruction([&]() { ++moved_from_destroyed; });
    cta.onDestruction([&]() { ++destroyed; });

    xtd::call_tracker::onMoveConstruction([&]() { ++move_constructed_a; });
    xtd::call_tracker::onCopyConstruction([&]() { ++copy_constructed_a; });
    xtd::optional<xtd::call_tracker> a = xtd::some(cta);

    xtd::call_tracker::onMoveConstruction([&]() { ++move_constructed_b; });
    xtd::call_tracker::onCopyConstruction([&]() { ++copy_constructed_b; });
    xtd::optional<xtd::call_tracker> b{a};

    xtd::call_tracker::onMoveConstruction([]() {});
    xtd::call_tracker::onCopyConstruction([]() {});
  }

  EXPECT_EQ(0, move_constructed_a);
  EXPECT_EQ(0, move_constructed_b);
  EXPECT_EQ(1, copy_constructed_a);
  EXPECT_EQ(1, copy_constructed_b);
  EXPECT_EQ(0, moved_from_destroyed);
  EXPECT_EQ(3, destroyed);
}

TEST(optional, copy) {
  int copied_into{0};
  int copied_to{0};

  int copied_into_new{0};
  int copy_constructed{0};

  int moved_from_destroyed{0};
  int destroyed_a{0};
  int destroyed_b{0};

  xtd::call_tracker::onCopyConstruction([&]() { ++copy_constructed; });

  {
    xtd::call_tracker cta, ctb;

    cta.onCopyingInto([&]() { ++copied_into; });
    cta.onCopyingIntoNew([&]() { ++copied_into_new; });
    cta.onMovedFromDestruction([&]() { ++moved_from_destroyed; });
    cta.onDestruction([&]() { ++destroyed_a; });

    ctb.onCopyAssign([&]() { ++copied_to; });
    ctb.onDestruction([&]() { ++destroyed_b; });

    auto a = xtd::some(cta);
    auto b = xtd::some(ctb);

    b = a;
  }

  xtd::call_tracker::onCopyConstruction([]() {});

  EXPECT_EQ(1, copied_into);
  EXPECT_EQ(1, copied_to);

  EXPECT_EQ(1, copied_into_new);
  EXPECT_EQ(2, copy_constructed);

  EXPECT_EQ(0, moved_from_destroyed);
  EXPECT_EQ(3, destroyed_a);
  EXPECT_EQ(1, destroyed_b);
}
