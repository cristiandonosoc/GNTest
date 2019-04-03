// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <warhol/graphics/common/render_command.h>

#include <third_party/catch2/catch.hpp>
#include <warhol/utils/log.h>

namespace warhol {
namespace test {

TEST_CASE("IndexRange") {
  SECTION("Range alone") {
    IndexRange range = 0;
    CHECK(GetOffset(range) == 0);

    range = PushOffset(range, 0x12345678);
    CHECK(GetOffset(range) == 0x12345678);

    range = PushOffset(range, 0x5678);
    CHECK(GetOffset(range) == 0x5678);

    range = PushOffset(range, 0x12345678deadbeef);
    CHECK(GetOffset(range) == 0xdeadbeef);
  }

  SECTION("Size alone") {
    IndexRange range = 0;
    CHECK(GetSize(range) == 0);

    range = PushSize(range, 0x12345678);
    CHECK(GetSize(range) == 0x12345678);

    range = PushSize(range, 0x5678);
    CHECK(GetSize(range) == 0x5678);

    range = PushSize(range, 0x12345678deadbeef);
    CHECK(GetSize(range) == 0xdeadbeef);
  }

  SECTION("Both") {
    IndexRange range = 0;

    range = PushOffset(range, 0x12345678);
    range = PushSize(range, 0xdeadbeef);
    CHECK(GetOffset(range) == 0x12345678);
    CHECK(GetSize(range) == 0xdeadbeef);
  }
}

}  // namespace test
}  // namespace warhol


