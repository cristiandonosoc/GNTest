// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

#include "warhol/memory/memory_pool.h"
#include "warhol/utils/log.h"

namespace warhol {

template <typename T>
struct List {
  uint32_t count = 0;

  struct Node {
    T value;
    Node* next = nullptr;
  };

  Node* head = nullptr;
  Node* tail = nullptr;
  MemoryPool pool = {};

  struct Iterator;
  Iterator begin() { return Iterator(head); }
  Iterator end() { return nullptr; }
};

template <typename T>
void Reset(List<T>* list) {
  list->count = 0;
  list->head = nullptr;
  list->tail = nullptr;
}

template <typename T>
List<T> CreateList(size_t bytes) {
  List<T> list = {};
  InitMemoryPool(&list.pool, bytes);
  return list;
}

template <typename T>
inline bool Empty(List<T>* list) {
  return list->count == 0;
}

// Will allocate into the pool first and then create a node into the list.
template <typename T>
T* Push(List<T>* list);

// Same as before but moving in a value.
template <typename T>
T* Push(List<T>* list, T t);

// *****************************************************************************
// Template Implementation
// *****************************************************************************

template <typename T>
void PushNode(List<T>* list, typename List<T>::Node* node);

// Will allocate into the pool first and then create a node into the list.
template <typename T>
T* Push(List<T>* list) {
  ASSERT(Valid(&list->pool));
  auto* node = Push<typename List<T>::Node>(&list->pool);
  PushNode(list, node);
  return &node->value;
}

template <typename T>
T* Push(List<T>* list, T t) {
  ASSERT(Valid(&list->pool));
  auto* node = Push<typename List<T>::Node>(&list->pool);
  PushNode(list, node);
  node->value = std::move(t);
  return &node->value;
}

template <typename T>
void PushNode(List<T>* list, typename List<T>::Node* node) {
  node->next = nullptr;
  if (Empty(list)) {
    list->head = node;
    list->tail = node;
    list->count++;
    return;
  }

  ASSERT(list->tail);
  list->tail->next = node;
  list->tail = node;
  list->count++;
}

// *****************************************************************************
// Iterator Implementation
// *****************************************************************************

template <typename T>
struct List<T>::Iterator {
  Iterator() = default;
  Iterator(typename List<T>::Node* t) : elem(t) {}

  DEFAULT_COPY_AND_ASSIGN(Iterator);
  DEFAULT_MOVE_AND_ASSIGN(Iterator);

  bool operator==(const Iterator& rhs) const { return elem == rhs.elem; }

  bool operator!=(const Iterator& rhs) const { return elem != rhs.elem; }

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

  typename List<T>::Node* elem = nullptr;
};

}  // namespace warhol
