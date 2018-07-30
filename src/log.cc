#include <stdio.h>

#include "log.h"

namespace warhol {

namespace {

inline void FileFlush(FILE* file, const std::string& msg) {
  fprintf(file, "%s\n", msg.c_str());
  fflush(file);
}

}  // namespace

void StdoutAndFlush(const std::string& msg) {
  FileFlush(stdout, msg);
}

void StderrAndFlush(const std::string& msg) {
  FileFlush(stderr, msg);
}

}   // namespace warhol
