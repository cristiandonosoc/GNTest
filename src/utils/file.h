// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

namespace warhol {

bool ReadWholeFile(const std::string& path, std::vector<char>* out);

}  // namespace warhol
