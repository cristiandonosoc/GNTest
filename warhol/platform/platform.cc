// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/platform/platform.h"

#include "warhol/sdl2/def.h"

namespace warhol {

uint64_t Platform::GetHighPerformanceCounter() {
  return SDL_GetPerformanceCounter();
}

}  // namespace warhol
