// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <warhol/containers/linked_list.h>

#include <third_party/catch2/catch.hpp>
#include <warhol/memory/memory_pool.h>

namespace warhol {
namespace test{

TEST_CASE("LinkedList") {
  SECTION("PushIntoList") {
    LinkedList<size_t> list;

    std::vector<size_t> values(5);
    std::vector<LinkNode<size_t>> nodes(5);
    for (size_t i = 0; i < nodes.size(); i++) {
      values[i] = i * i;
      nodes[i].value = values[i];
      PushIntoList(&list, &nodes[i]);
    }

    size_t i = 0;
    for (size_t& element : list) {
      CHECK(element == values[i]);
      i++;
    }
  }

  SECTION("PushIntoListFromMemoryPool Without Value") {
    MemoryPool pool;
    InitMemoryPool(&pool, KILOBYTES(1));

    LinkedList<size_t> list;

    std::vector<size_t> values(5);
    for (size_t i = 0; i < values.size(); i++) {
      size_t* val = PushIntoListFromMemoryPool(&list, &pool);
      values[i] = i * i;
      *val = values[i];
    }

    size_t i = 0;
    for (size_t& element : list) {
      CHECK(element == values[i]);
      i++;
    }
  }

  SECTION("PushIntoListFromMemoryPool With Value") {
    MemoryPool pool;
    InitMemoryPool(&pool, KILOBYTES(1));

    LinkedList<size_t> list;
    std::vector<size_t> values(5);
    for (size_t i = 0; i < values.size(); i++) {
      values[i] = i * i + 1;
      size_t* val = PushIntoListFromMemoryPool(&list, &pool, values[i]);
      CHECK(*val == values[i]);
    }

    size_t i = 0;
    for (size_t& element : list) {
      CHECK(element == values[i]);
      i++;
    }
  }
}

}  // namespace test
}  // namespace warhol
