// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

#include "utils/status.h"

namespace warhol {

Status ReadWholeFile(const std::string& path, std::vector<char>* out);

}  // namespace warhol
