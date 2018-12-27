// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <vector>

#include <warhol/memory/array_heap.h>
#include <warhol/utils/log.h>
#include <warhol/utils/macros.h>

using namespace warhol;

template <typename T>
std::string UsedIndexes(const T& array_heap) {
  std::vector<size_t> indexes;
  for (size_t i = 0; i < ARRAY_SIZE(array_heap.used); i++) {
    if (array_heap.used[i])
      indexes.push_back(i);
  }

  std::stringstream ss;
  for (int i : indexes)
    ss << i << ", ";

  return ss.str();
}



int main() {
}
