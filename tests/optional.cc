// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <vector>

#include <third_party/catch2/catch.hpp>
#include <warhol/utils/optional.h>

using namespace warhol;

namespace {

struct Counter {
  int ctor_called = 0;
  int dtor_called = 0;
  int copy_ctor_called = 0;
  int copy_assign_called = 0;
  int move_ctor_called = 0;
  int move_assign_called = 0;
};

Counter gCounter = {};
void ClearCounters() {
  gCounter = {};
}

#define CHECK_COUNTERS(c, d, cc, ca, mc, ma) \
  REQUIRE(gCounter.ctor_called == c);\
  REQUIRE(gCounter.dtor_called == d);\
  REQUIRE(gCounter.copy_ctor_called == cc);\
  REQUIRE(gCounter.copy_assign_called == ca);\
  REQUIRE(gCounter.move_ctor_called == mc);\
  REQUIRE(gCounter.move_assign_called == ma);\

struct Foo {
  Foo() {
    gCounter.ctor_called++;
  }
  ~Foo() {
    gCounter.dtor_called++;
  }
  Foo(const Foo& rhs) {
    a = rhs.a;
    gCounter.copy_ctor_called++;
  }
  Foo& operator=(const Foo& rhs) {
    gCounter.copy_assign_called++;
    a = rhs.a;
    return *this;
  }
  Foo(Foo&& rhs) {
    a = rhs.a;
    rhs.a = 0;
    gCounter.move_ctor_called++;
  }
  Foo& operator=(Foo&& rhs) {
    a = rhs.a;
    rhs.a = 0;
    gCounter.move_assign_called++;
    return *this;
  }

  int a = 0;
};

constexpr int kValue = 10;

TEST_CASE("Constructors") {
  Foo foo = {};
  foo.a = kValue;

  // Empty.
  ClearCounters();
  Optional<Foo> o1;
  REQUIRE(!o1.has_value());
  CHECK_COUNTERS(0, 0, 0, 0, 0, 0);

  // Implicit move.
  ClearCounters();
  Optional<Foo> o2(foo);
  REQUIRE(o2.has_value());
  REQUIRE(o2->a == kValue);
  CHECK_COUNTERS(0, 0, 0, 1, 0, 0);

  // Copy.
  ClearCounters();
  Optional<Foo> o3 = *o2;
  REQUIRE(o3.has_value());
  REQUIRE(o3->a == kValue);
  CHECK_COUNTERS(0, 0, 0, 1, 0, 0);

  // Move.
  ClearCounters();
  Optional<Foo> o4(std::move(foo));
  REQUIRE(o4.has_value());
  REQUIRE(o4->a == kValue);
  CHECK_COUNTERS(0, 0, 0, 0, 0, 1);

  ClearCounters();
  Optional<Foo> o5(o4);
  REQUIRE(o4.has_value());
  REQUIRE(o5.has_value());
  REQUIRE(o4->a == kValue);
  REQUIRE(o5->a == kValue);
  CHECK_COUNTERS(0, 0, 0, 1, 0, 0);

  ClearCounters();
  Optional<Foo> o6(std::move(o5));
  REQUIRE(!o5.has_value());
  REQUIRE(o6.has_value());
  REQUIRE(o6->a == kValue);
  CHECK_COUNTERS(0, 0, 0, 0, 0, 1);

  ClearCounters();
  Optional<Foo> o7 = o6;
  REQUIRE(o6.has_value());
  REQUIRE(o7.has_value());
  REQUIRE(o6->a == kValue);
  REQUIRE(o7->a == kValue);
  CHECK_COUNTERS(0, 0, 0, 1, 0, 0);

  ClearCounters();
  Optional<Foo> o8 = std::move(o7);
  REQUIRE(!o7.has_value());
  REQUIRE(o8.has_value());
  REQUIRE(o8->a == kValue);
  CHECK_COUNTERS(0, 0, 0, 0, 0, 1);
}

TEST_CASE("Destructors") {
  Foo foo;
  foo.a = kValue;

  ClearCounters();
  { Optional<Foo> o1; }
  CHECK_COUNTERS(0, 0, 0, 0, 0, 0);

  ClearCounters();
  { Optional<Foo> o1 = foo; }
  CHECK_COUNTERS(0, 1, 0, 1, 0, 0);

  ClearCounters();
  {
    Optional<Foo> o1 = foo;
    Optional<Foo> o2 = std::move(o1);
    Optional<Foo> o3 = std::move(o2);
    Optional<Foo> o4 = std::move(o3);
  }
  CHECK_COUNTERS(0, 1, 0, 1, 0, 3);
}

}  // namespace
