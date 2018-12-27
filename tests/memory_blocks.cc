// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <third_party/catch2/catch.hpp>

#include <warhol/memory/array_heap.h>

using namespace warhol;

namespace {

template <typename T>
std::vector<int> GetUsedIndexes(const T& array_heap) {
  std::vector<int> indexes;
  for (int i = 0; i < ARRAY_SIZE(array_heap.used); i++) {
    if (array_heap.used[i])
      indexes.push_back(i);
  }

  return indexes;
}

}  // namespace

#define TOKEN_PASTE(x, y) x##y
#define CAT(x,y) TOKEN_PASTE(x,y)
#define ALLOCD(heap_ptr) auto CAT(block, __LINE__) = heap_ptr->AllocateBlock();

TEST_CASE("Allocate blocks") {
  auto array_heap = std::make_unique<ArrayHeap<MemoryBlockSize::k256, 10>>();

  auto block = array_heap->AllocateBlock();
  REQUIRE(GetUsedIndexes(*array_heap).size() == 1);

  auto block2 = array_heap->AllocateBlock();
  REQUIRE(GetUsedIndexes(*array_heap).size() == 2);
  REQUIRE(block.data != block2.data);
  REQUIRE(block.data == (uint8_t*)&array_heap->blocks[0]);
  REQUIRE(block2.data == (uint8_t*)&array_heap->blocks[1]);

  block.Free();
  REQUIRE(GetUsedIndexes(*array_heap).size() == 1);

  {
    auto block = array_heap->AllocateBlock();
    REQUIRE(GetUsedIndexes(*array_heap).size() == 2);
  }
  REQUIRE(GetUsedIndexes(*array_heap).size() == 1);

  ALLOCD(array_heap);
  ALLOCD(array_heap);
  ALLOCD(array_heap);
  ALLOCD(array_heap);
  REQUIRE(GetUsedIndexes(*array_heap).size() == 5);
}

