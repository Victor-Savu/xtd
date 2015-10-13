#pragma once

#include <functional>
#include <utility>

namespace xtd {

namespace {
class call_tracker {
 public:
  using fn = std::function<void()>;

  call_tracker() { defaultConstruction(); }

  ~call_tracker() { destruction(); }

  call_tracker(call_tracker const& ct) {
    copyConstruction();
    ct.copyingIntoNew();

    destruction = ct.destruction;
    movedFromDestruction = ct.movedFromDestruction;

    copyingIntoNew = ct.copyingIntoNew;
    movingIntoNew = ct.movingIntoNew;

    copyAssign = ct.copyAssign;
    copyingInto = ct.copyingInto;
    selfAssign = ct.selfAssign;

    moveAssign = ct.moveAssign;
    movingInto = ct.movingInto;
    selfMove = ct.selfMove;
  }

  call_tracker(call_tracker&& ct) {
    moveConstruction();
    ct.movingIntoNew();

    destruction = std::move(ct.destruction);
    ct.destruction = movedFromDestruction = std::move(ct.movedFromDestruction);

    copyingIntoNew = std::move(ct.copyingIntoNew);
    movingIntoNew = std::move(ct.movingIntoNew);

    copyAssign = std::move(ct.copyAssign);
    copyingInto = std::move(ct.copyingInto);
    selfAssign = std::move(ct.selfAssign);

    moveAssign = std::move(ct.moveAssign);
    movingInto = std::move(ct.movingInto);
    selfMove = std::move(ct.selfMove);
  }

  call_tracker& operator=(call_tracker const& ct) {
    if (&ct == this) {
      selfAssign();
    } else {
      copyAssign();
      ct.copyingInto();

      destruction = ct.destruction;
      movedFromDestruction = ct.movedFromDestruction;

      copyingIntoNew = ct.copyingIntoNew;
      movingIntoNew = ct.movingIntoNew;

      copyAssign = ct.copyAssign;
      copyingInto = ct.copyingInto;
      selfAssign = ct.selfAssign;

      moveAssign = ct.moveAssign;
      movingInto = ct.movingInto;
      selfMove = ct.selfMove;
    }
    return *this;
  }

  call_tracker& operator=(call_tracker&& ct) {
    if (&ct == this) {
      selfMove();
    } else {
      moveAssign();
      ct.movingInto();

      destruction = std::move(ct.destruction);
      ct.destruction = movedFromDestruction = std::move(ct.movedFromDestruction);

      copyingIntoNew = std::move(ct.copyingIntoNew);
      movingIntoNew = std::move(ct.movingIntoNew);

      copyAssign = std::move(ct.copyAssign);
      copyingInto = std::move(ct.copyingInto);
      selfAssign = std::move(ct.selfAssign);

      moveAssign = std::move(ct.moveAssign);
      movingInto = std::move(ct.movingInto);
      selfMove = std::move(ct.selfMove);
    }
    return *this;
  }

  template <typename Fn>
  static void onDefaultConstruction(Fn&& f) {
    defaultConstruction = std::forward<Fn>(f);
  }
  template <typename Fn>
  static void onCopyConstruction(Fn&& f) {
    copyConstruction = std::forward<Fn>(f);
  }
  template <typename Fn>
  static void onMoveConstruction(Fn&& f) {
    moveConstruction = std::forward<Fn>(f);
  }

  template <typename Fn>
  auto& onDestruction(Fn&& f) {
    destruction = std::forward<Fn>(f);
    return *this;
  }
  template <typename Fn>
  auto& onMovedFromDestruction(Fn&& f) {
    movedFromDestruction = std::forward<Fn>(f);
    return *this;
  }

  template <typename Fn>
  auto& onCopyAssign(Fn&& f) {
    copyAssign = std::forward<Fn>(f);
    return *this;
  }
  template <typename Fn>
  auto& onCopyingIntoNew(Fn&& f) {
    copyingIntoNew = std::forward<Fn>(f);
    return *this;
  }
  template <typename Fn>
  auto& onCopyingInto(Fn&& f) {
    copyingInto = std::forward<Fn>(f);
    return *this;
  }
  template <typename Fn>
  auto& onSelfAssign(Fn&& f) {
    selfAssign = std::forward<Fn>(f);
    return *this;
  }

  template <typename Fn>
  auto& onMoveAssign(Fn&& f) {
    moveAssign = std::forward<Fn>(f);
    return *this;
  }
  template <typename Fn>
  auto& onMovingIntoNew(Fn&& f) {
    movingIntoNew = std::forward<Fn>(f);
    return *this;
  }
  template <typename Fn>
  auto& onMovingInto(Fn&& f) {
    movingInto = std::forward<Fn>(f);
    return *this;
  }
  template <typename Fn>
  auto& onSelfMove(Fn&& f) {
    selfMove = std::forward<Fn>(f);
    return *this;
  }

 private:
  static fn defaultConstruction;
  static fn copyConstruction;
  static fn moveConstruction;

  fn destruction{[]() {}};
  fn movedFromDestruction{[](){}};

  fn copyingIntoNew{[](){}};
  fn movingIntoNew{[](){}};

  fn copyAssign{[](){}};
  fn copyingInto{[](){}};
  fn selfAssign{[](){}};

  fn moveAssign{[](){}};
  fn movingInto{[](){}};
  fn selfMove{[](){}};
};

call_tracker::fn call_tracker::defaultConstruction{[]() {}};
call_tracker::fn call_tracker::copyConstruction{[]() {}};
call_tracker::fn call_tracker::moveConstruction{[]() {}};
}
}
