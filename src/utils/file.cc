// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/utils/file.h"

#include <stdlib.h>

#include "src/utils/log.h"

namespace warhol {

bool ReadWholeFile(const std::string& path, std::vector<char>* out) {
  FILE* file;
  size_t file_size;

  file = fopen(path.data(), "rb");
  if (file == NULL) {
    LOG(ERROR) << "Could not open file: " << path.data();
    return false;
  }

  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  out->clear();
  out->resize(file_size + 1);
  auto result = fread(out->data(), 1, file_size, file);
  if (result != file_size) {
    LOG(ERROR) << "Could not read file: " << path.data();
    return false;
  }

  fclose(file);
  out->back() = '\0';

  return true;
}

}  // namespace warhol
