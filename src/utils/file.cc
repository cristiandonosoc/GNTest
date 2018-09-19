// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdlib.h>

#include "utils/file.h"

namespace warhol {

Status
ReadWholeFile(const std::string& path, std::vector<char>* out) {
  FILE* file;
  size_t file_size;

  file = fopen(path.data(), "rb");
  if (file == NULL)
    return Status("Could not open file: %s", path.data());

  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  out->clear();
  out->resize(file_size);
  auto result = fread(out->data(), 1, file_size, file);
  if (result != file_size)
    return Status("Could not read file: %s", path.data());

  fclose(file);
  return Status::Ok();
}

}  // namespace warhol