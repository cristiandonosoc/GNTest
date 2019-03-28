// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <warhol/memory/memory_pool.h>

#include <third_party/catch2/catch.hpp>

namespace warhol {
namespace test{

TEST_CASE("MemoryPool") {
  SECTION("PushIntoPool") {
    MemoryPool pool;
    InitMemoryPool(&pool, KILOBYTES(1));

    std::vector<int> values;
    std::vector<int*> pointers;
    for (int i = 0; i < 10; i++) {
      pointers.push_back(PushIntoPool<int>(&pool));
      values.push_back(i * i);
      *pointers.back() = values.back();
    }

    int* ptr = (int*)pool.data.get();
    for (size_t i = 0; i < values.size(); i++) {
      CHECK(*ptr++ == values[i]);
    }
  }
}

}  // namespace test
}  // namespace warhol
