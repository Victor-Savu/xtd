#include <xtd/call_tracker.hh>

#include <gtest/gtest.h>

TEST(call_tracker, constructor) {
  int constructor_called{0};
  xtd::call_tracker::onDefaultConstruction([&]() { ++constructor_called; });

  xtd::call_tracker a;

  EXPECT_EQ(1, constructor_called);

  xtd::call_tracker::onDefaultConstruction([]() {});
}

TEST(call_tracker, copy_constructor) {
  int copy_constructor_called{0};
  int copying_into_new_called{0};
  xtd::call_tracker::onCopyConstruction([&]() { ++copy_constructor_called; });

  xtd::call_tracker a;
  a.onCopyingIntoNew([&]() { ++copying_into_new_called; });

  xtd::call_tracker b{a};

  EXPECT_EQ(1, copy_constructor_called);
  EXPECT_EQ(1, copying_into_new_called);

  xtd::call_tracker::onCopyConstruction([]() {});
}

TEST(call_tracker, move_constructor) {
  int move_constructor_called{0};
  int moving_into_new_called{0};
  xtd::call_tracker::onMoveConstruction([&]() { ++move_constructor_called; });

  EXPECT_EQ(0, move_constructor_called);
  EXPECT_EQ(0, moving_into_new_called);

  xtd::call_tracker a;

  EXPECT_EQ(0, move_constructor_called);
  EXPECT_EQ(0, moving_into_new_called);

  a.onMovingIntoNew([&]() { ++moving_into_new_called; });

  EXPECT_EQ(0, move_constructor_called);
  EXPECT_EQ(0, moving_into_new_called);

  xtd::call_tracker b{std::move(a)};

  EXPECT_EQ(1, move_constructor_called);
  EXPECT_EQ(1, moving_into_new_called);

  xtd::call_tracker::onMoveConstruction([]() {});
}

TEST(call_tracker, destruction) {
  int destructor_called{0};
  {
    xtd::call_tracker a;
    EXPECT_EQ(0, destructor_called);
    a.onDestruction([&]() { ++destructor_called; });
    EXPECT_EQ(0, destructor_called);
  }
  EXPECT_EQ(1, destructor_called);
}

TEST(call_tracker, copy_assign) {
  int copy_assign_called{0};
  int copying_into_called{0};

  xtd::call_tracker a;
  a.onCopyingInto([&]() { ++copying_into_called; });

  xtd::call_tracker b;
  b.onCopyAssign([&]() { ++copy_assign_called; });

  b = a;

  EXPECT_EQ(1, copy_assign_called);
  EXPECT_EQ(1, copying_into_called);
}

TEST(call_tracker, self_assign) {
  int self_assign_called{0};

  xtd::call_tracker a;
  a.onSelfAssign([&]() { ++self_assign_called; });

  a = a;

  EXPECT_EQ(1, self_assign_called);
}

TEST(call_tracker, move_assign) {
  int move_assign_called{0};
  int moving_into_called{0};

  xtd::call_tracker a;
  a.onMovingInto([&]() { ++moving_into_called; });

  xtd::call_tracker b;
  b.onMoveAssign([&]() { ++move_assign_called; });

  b = std::move(a);

  EXPECT_EQ(1, move_assign_called);
  EXPECT_EQ(1, moving_into_called);
}

TEST(call_tracker, self_move) {
  int self_move_called{0};

  xtd::call_tracker a;
  a.onSelfMove([&]() { ++self_move_called; });

  a = std::move(a);

  EXPECT_EQ(1, self_move_called);
}

TEST(call_tracker, moved_from_destruction) {
  int moved_from_destruction{0};
  int destruction{0};

  {
    xtd::call_tracker b;

    {
      xtd::call_tracker a;
      a.onDestruction([&]() { ++destruction; });
      a.onMovedFromDestruction([&]() { ++moved_from_destruction; });

      b = std::move(a);
    }

    EXPECT_EQ(1, moved_from_destruction);
    EXPECT_EQ(0, destruction);
  }

  EXPECT_EQ(1, moved_from_destruction);
  EXPECT_EQ(1, destruction);
}
