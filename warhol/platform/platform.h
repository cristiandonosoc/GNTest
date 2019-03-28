// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

namespace warhol {

// The platform is meant to give an interface to OS specific functionality
// in an uniform way. Each platform should compile its own implementation of
// this interface. The implementations are in warhol/arch/*_platform.cc

std::string GetBasePath();
std::string GetCurrentExecutablePath();
std::string GetCurrentExecutableDirectory();

uint64_t GetNanoseconds();

}  // namespace warhol
