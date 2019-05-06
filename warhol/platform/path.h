// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

namespace warhol {

// Returns a concatenate of path parts joined together (normally by '/').
std::string PathJoin(std::vector<std::string_view> paths);

// Strip everything up to and including the last slash.
std::string GetBasename(const std::string& path);

}  // namespace warhol
