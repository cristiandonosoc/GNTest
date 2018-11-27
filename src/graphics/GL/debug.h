// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdint.h>

namespace warhol {

// Returns the GL error enum transformated to int (so that we don't need to add
// the definition here). If there was an error, the transformated name of the
// error will be put into |error_name|.
uint32_t GetGLError(const char** error_name);

}  // namespace warhol
