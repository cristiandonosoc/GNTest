// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include "warhol/memory/memory_pool.h"
#include "warhol/utils/assert.h"

namespace warhol {

template <typename T>
struct LinkNode {
  T value;
  LinkNode* next = nullptr;
};

template <typename T>
struct LinkedList {
  uint32_t count = 0;

  LinkNode<T>* head = nullptr;
  LinkNode<T>* tail = nullptr;

  struct Iterator {
    Iterator() = default;
    Iterator(LinkNode<T>* t) : elem(t) {}

    DEFAULT_COPY_AND_ASSIGN(Iterator);
    DEFAULT_MOVE_AND_ASSIGN(Iterator);

    bool operator==(const Iterator& rhs) const {
      return elem == rhs.elem;
    }

    bool operator!=(const Iterator& rhs) const {
      return elem != rhs.elem;
    }

    Iterator& operator++() {
      elem = elem->next;
      return *this;
    }

    Iterator operator++(int) {
      elem = elem->next;
      return Iterator(elem->next);
    }

    T* operator->() { return &elem->value; }
    T& operator*() { return elem->value; }

    LinkNode<T>* elem = nullptr;
  };

  Iterator begin() { return Iterator(head); }
  Iterator end() { return nullptr; }
};

template <typename T>
void PushIntoList(LinkedList<T>* list, LinkNode<T>* node) {
  (void)list;
  (void)node;
  NOT_IMPLEMENTED();
}

// Will allocate into the pool first and then create a node into the list.
template <typename T>
T* PushIntoListFromPool(LinkedList<T>* list, MemoryPool* pool) {
  (void)list;
  (void)pool;
  NOT_IMPLEMENTED();
  return nullptr;
}

}  // namespace warhol
