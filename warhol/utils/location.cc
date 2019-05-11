// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/utils/location.h"

#include <assert.h>
#include <stdio.h>

#include <utility>
#include <vector>

#include "warhol/platform/path.h"
#include "warhol/utils/log.h"
#include "warhol/utils/string.h"

namespace warhol {
namespace {

LocationStack* GetPerThreadLocationStack() {
  thread_local LocationStack stack;
  return &stack;
}

void PushStackLocation(LocationScope* scope) {
  LocationStack* stack = GetPerThreadLocationStack();
  ASSERT(stack->size < (int)LocationStack::kMaxLocationStackSize - 1);
  stack->entries[stack->size++] = {scope};
}

void PopStackLocation() {
  LocationStack* stack = GetPerThreadLocationStack();
  ASSERT(stack->size > 0);
  stack->size--;
}

}  // namespace

LocationStack* GetLocationStack() {
  return GetPerThreadLocationStack();
}

std::string LocationStackToString(LocationStack* stack) {
  std::vector<std::string> log_stack;
  log_stack.reserve(stack->size);
  for (int i = stack->size - 1; i >= 0; i--) {
    LocationStack::Entry& entry = stack->entries[i];

    const Location& loc = entry.scope->location;
    std::string line = StringPrintf("%.2d. [%s:%d][%s] %s\n",
                                    stack->size - i - 1,
                                    GetBasename(loc.file).c_str(),
                                    loc.line,
                                    loc.function,
                                    entry.scope->stream_.str().c_str());
    log_stack.push_back(std::move(line));
  }

  return Join(log_stack);
}

LocationScope::LocationScope(Location location) : location(location) {
  PushStackLocation(this);
}

LocationScope::~LocationScope() {
  PopStackLocation();
}

}  // namespace warhol
