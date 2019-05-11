// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <warhol/containers/list.h>

#include <third_party/catch2/catch.hpp>
#include <warhol/memory/memory_pool.h>

namespace warhol {
namespace test{

TEST_CASE("List") {
  SECTION("PushIntoListFromMemoryPool Without Value") {
    MemoryPool pool;

    auto list = CreateList<size_t>(KILOBYTES(2));

    std::vector<size_t> values(5);
    for (size_t i = 0; i < values.size(); i++) {
      size_t* val = Push(&list);
      values[i] = i * i;
      *val = values[i];
    }

    size_t i = 0;
    for (size_t& element : list) {
      CHECK(element == values[i]);
      i++;
    }

    CHECK(list.count == 5);
  }

  SECTION("PushIntoListFromMemoryPool With Value") {
    MemoryPool pool;

    auto list = CreateList<size_t>(KILOBYTES(2));
    std::vector<size_t> values(5);
    for (size_t i = 0; i < values.size(); i++) {
      values[i] = i * i + 1;
      size_t* val = Push(&list, values[i]);
      CHECK(*val == values[i]);
    }

    size_t i = 0;
    for (size_t& element : list) {
      CHECK(element == values[i]);
      i++;
    }

    CHECK(list.count == 5);
  }
}

}  // namespace test
}  // namespace warhol
