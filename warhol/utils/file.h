// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <string>
#include <vector>

#include "warhol/utils/assert.h"
#include "warhol/utils/clear_on_move.h"

namespace warhol {

// Reads a complete file as binary data into |out|.
// If |add_extra_zero| is true, it will append a '\0' character to the end of
// the data.
bool ReadWholeFile(const std::string& path,
                   std::string* out,
                   bool add_extra_zero = true);

struct FileHandle {
  RAII_CONSTRUCTORS(FileHandle);
  ClearOnMove<void*> hndl;
};

inline bool Valid(FileHandle* file) { return file->hndl.has_value(); }

FileHandle OpenFile(const std::string& path, bool append = false);
void WriteToFile(FileHandle*, void* data, size_t size);
void CloseFile(FileHandle*);


}  // namespace warhol
