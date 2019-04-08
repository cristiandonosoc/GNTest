// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/utils/macros.h"

namespace warhol {
namespace imgui {

// Runs Imgui::Begin/Imgui::End
struct ImguiWindow {
  ImguiWindow(const char* name);
  ~ImguiWindow();
  DELETE_COPY_AND_ASSIGN(ImguiWindow);
  DELETE_MOVE_AND_ASSIGN(ImguiWindow);
};

}  // namespace imgui
}  // namespace warhol
