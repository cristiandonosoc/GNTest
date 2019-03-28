// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/platform/platform.h"

#include <time.h>
#include <string.h>
#include <unistd.h>

#include "warhol/utils/log.h"
#include "warhol/utils/path.h"

namespace warhol {

std::string GetCurrentExecutablePath() {
  char buf[1024];
  int res = readlink("/proc/self/exe", buf, sizeof(buf));
  if (res < 0) {
    LOG(ERROR) << "Could not get path to current executable: "
               << strerror(errno);
    return std::string();
  }

  return buf;
}

std::string GetCurrentExecutableDirectory() {
  std::string exe_path = GetCurrentExecutablePath();
  size_t separator = exe_path.rfind('/');
  if (separator == std::string::npos)
    return exe_path;
  return exe_path.substr(0, separator);
}

std::string GetBasePath() {
  std::string exe_path = GetCurrentExecutablePath();
  size_t separator = exe_path.rfind('/');
  if (separator == std::string::npos)
    return exe_path;
  auto base_path = exe_path.substr(0, separator);
  return PathJoin({std::move(base_path), ".."});
}

uint64_t GetNanoseconds() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now.tv_sec * 1000000000 + now.tv_nsec;
}

}  // namespace warhol
