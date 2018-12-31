// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/platform/platform.h"

#include "warhol/sdl2/def.h"

namespace warhol {

std::string Platform::GetCurrentExecutableDirectory() {
  std::string exe_path = GetCurrentExecutablePath();
  size_t separator = exe_path.rfind('/');
  if (separator == std::string::npos)
    return exe_path;
  return exe_path.substr(0, separator);
}

uint64_t Platform::GetHighPerformanceCounter() {
  return SDL_GetPerformanceCounter();
}

}  // namespace warhol
