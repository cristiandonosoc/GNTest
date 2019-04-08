// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "warhol/ui/imgui/helpers.h"

#include "warhol/ui/imgui/def.h"

namespace warhol {
namespace imgui {

ImguiWindow::ImguiWindow(const char* name) {
  ImGui::Begin(name);
}

ImguiWindow::~ImguiWindow() {
  ImGui::End();
}

}  // namespace imgui
}  // namespace warhol
