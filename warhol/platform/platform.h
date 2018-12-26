// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

namespace warhol {

// The platform is meant to give an interface to OS specific functionality
// in an uniform way. Each platform should compile its own implementation of
// this interface. The implementations are in warhol/arch/*_platform.cc
class Platform {
 public:
   // Returns the path of the current link. Empty on error.
   static std::string GetCurrentExecutablePath();

   // Returns the path to base of the project.
   static std::string GetBasePath();

   static uint64_t GetHighPerformanceCounter();
};


}  // namespace warhol
