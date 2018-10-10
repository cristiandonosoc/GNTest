// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

namespace warhol {
namespace arch {

// The ArchProvider is meant to give an interface to OS specific functionality
// in an uniform way. Each platform should compile its own implementation of
// this interface. The implementations are in src/arch/*_arch_provider.cc
class ArchProvider {
 public:
   // Returns the path of the current link. Empty on error.
   static std::string GetCurrentExecutablePath();

   // Returns the path to base of the project.
   static std::string GetBasePath();
};

}  // namespace arch
}  // namespace warhol
