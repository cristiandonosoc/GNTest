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

  // ***** Container-ish API.

  struct ForwardIterator;

  ForwardIterator begin() { return ForwardIterator(head); }
  ForwardIterator end() { return nullptr; }

  size_t size() const { return count; }
  bool empty() const { return count == 0; }

  // ***** ForwardIterator Implementation.

  struct ForwardIterator {
    ForwardIterator() = default;
    ForwardIterator(LinkNode<T>* t) : elem(t) {}

    DEFAULT_COPY_AND_ASSIGN(ForwardIterator);
    DEFAULT_MOVE_AND_ASSIGN(ForwardIterator);

    bool operator==(const ForwardIterator& rhs) const {
      return elem == rhs.elem;
    }

    bool operator!=(const ForwardIterator& rhs) const {
      return elem != rhs.elem;
    }

    ForwardIterator& operator++() {
      elem = elem->next;
      return *this;
    }

    ForwardIterator operator++(int) {
      elem = elem->next;
      return ForwardIterator(elem->next);
    }

    T* operator->() { return &elem->value; }
    T& operator*() { return elem->value; }

    LinkNode<T>* elem = nullptr;
  };

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
