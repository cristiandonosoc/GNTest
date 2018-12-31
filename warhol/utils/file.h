// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

namespace warhol {

// Reads a complete file as binary data into |out|.
// If |add_extra_zero| is true, it will append a '\0' character to the end of
// the data.
bool ReadWholeFile(const std::string& path,
                   std::vector<char>* out,
                   bool add_extra_zero = true);

}  // namespace warhol
