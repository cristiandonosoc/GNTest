// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/platform/platform.h"

#include <string.h>
#include <unistd.h>

#include "warhol/utils/log.h"
#include "warhol/utils/path.h"

namespace warhol {

std::string Platform::GetCurrentExecutablePath() {
  char buf[1024];
  int res = readlink("/proc/self/exe", buf, sizeof(buf));
  if (res < 0) {
    LOG(ERROR) << "Could not get path to current executable: "
               << strerror(errno);
    return std::string();
  }

  return buf;
}

std::string Platform::GetCurrentExecutableDirectory() {
  std::string exe_path = GetCurrentExecutablePath();
  size_t separator = exe_path.rfind('/');
  if (separator == std::string::npos)
    return exe_path;
  return exe_path.substr(0, separator);
}

std::string Platform::GetBasePath() {
  std::string exe_path = GetCurrentExecutablePath();
  size_t separator = exe_path.rfind('/');
  if (separator == std::string::npos)
    return exe_path;
  auto base_path = exe_path.substr(0, separator);
  return PathJoin({std::move(base_path), ".."});
}

}  // namespace warhol
