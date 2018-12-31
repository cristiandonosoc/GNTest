// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/utils/file.h"

#include <stdlib.h>

#include "warhol/utils/log.h"

namespace warhol {

bool ReadWholeFile(const std::string& path,
                   std::vector<char>* out,
                   bool add_extra_zero) {
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
  int pad = add_extra_zero ? 1 : 0;
  out->resize(file_size + pad);
  auto result = fread(out->data(), 1, file_size, file);
  if (result != file_size) {
    LOG(ERROR) << "Could not read file: " << path.data();
    return false;
  }

  fclose(file);
  if (add_extra_zero)
    out->back() = '\0';

  return true;
}

}  // namespace warhol
