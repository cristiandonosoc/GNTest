// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/assets.h"

#include "src/arch/arch_provider.h"
#include "src/utils/path.h"

namespace warhol {

std::string
Assets::ShaderPath(std::string shader_name) {
  return PathJoin({arch::ArchProvider::GetBasePath(),
                   "shaders",
                   std::move(shader_name)});
}

}  // namespace warhol
