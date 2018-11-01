// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/arch/arch_provider.h"

#include <assert.h>

namespace warhol {
namespace arch {

std::string ArchProvider::GetCurrentExecutablePath() {
  // TODO(Cristian): Not implemented.
  assert(false);
  return "";
}

std::string ArchProvider::GetBasePath() {
  // TODO(Cristian): Not implemented.
  assert(false);
  return "";
}

}  // namespace arch
}  // namespace warhol
