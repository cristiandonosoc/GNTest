// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>

#include "warhol/graphics/common/renderer.h"

namespace warhol {

std::string GetTexturePath(const std::string& texture_name);

std::string GetShaderPath(const std::string& shader_name, RendererType);

}  // namespace warhol
