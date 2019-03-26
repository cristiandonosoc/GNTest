// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/platform/platform.h"

#include <mach-o/dyld.h>
#include <sys/errno.h>

#include "warhol/utils/assert.h"
#include "warhol/utils/log.h"
#include "warhol/utils/path.h"

namespace warhol {

std::string GetCurrentExecutablePath() {
  char buf[256];
  uint32_t bufsize = sizeof(buf);
  int res = _NSGetExecutablePath(buf, &bufsize);
  if (res != 0) {
    LOG(ERROR) << "Could not get path to current executable: "
               << strerror(errno);
    return {};
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

#error GetPerformanceCounter not imlemented.
#error GetPerformanceFrequency not implemented.

}  // namespace warhol
