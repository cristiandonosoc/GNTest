// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/platform/platform.h"

#include "warhol/window/sdl/def.h"

namespace warhol {

uint64_t Platform::GetHighPerformanceCounter() {
#ifndef WARHOL_SDL_ENABLED
#error No defined performance counter backend
#else
  return SDL_GetPerformanceCounter();
#endif
}

uint64_t Platform::GetHighPerformanceFrequency() {
#ifndef WARHOL_SDL_ENABLED
#error No defined performance counter backend
#else
  return SDL_GetPerformanceCounter();
#endif
}

}  // namespace warhol
