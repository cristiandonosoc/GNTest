// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/arch/arch_provider.h"

#include <string.h>
#include <unistd.h>

#include "src/utils/log.h"
#include "src/utils/path.h"

namespace warhol {
namespace arch {

std::string ArchProvider::GetCurrentExecutablePath() {
  char buf[1024];
  int res = readlink("/proc/self/exe", buf, sizeof(buf));
  if (res < 0) {
    LOG(ERROR) << "Could not get path to current executable: "
               << strerror(errno);
    return std::string();
  }

  return buf;
}

std::string ArchProvider::GetBasePath() {
  std::string exe_path = GetCurrentExecutablePath();
  size_t separator = exe_path.rfind('/');
  if (separator == std::string::npos)
    return exe_path;
  auto base_path = exe_path.substr(0, separator);
  return PathJoin({std::move(base_path), ".."});
}

}  // namespace arch
}  // namespace warhol
