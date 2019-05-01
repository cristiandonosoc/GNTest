// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/utils/path.h"

#include "warhol/utils/string.h"

namespace warhol {

// TODO(Cristian): Use std::filesystem (C++17) for this eventually.
std::string PathJoin(std::vector<std::string_view> paths) {
  std::vector<std::string> pieces;
  pieces.reserve(2 * paths.size() - 1);
  for (size_t i = 0; i < paths.size(); i++) {
    pieces.emplace_back(std::move(paths[i]));
    // We add a "/".
    if (i < paths.size() - 1)
      pieces.emplace_back("/");
  }

  return Concatenate(std::move(pieces));
}

std::string GetBasename(const std::string& path) {
  size_t separator = path.rfind('/');
  if (separator == std::string::npos)
    return path;
  return path.substr(separator + 1);
}

}  // namespace warhol

