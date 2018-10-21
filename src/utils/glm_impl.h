// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

// Same as glm.h, only that brings the whole implementation (this is normally
// what you need if you're coding an implementation aka a .cc file).

#include "src/utils/macros.h"

BEGIN_IGNORE_WARNINGS()
#include <third_party/include/glm/glm.hpp>
#include <third_party/include/glm/gtc/matrix_transform.hpp>
#include <third_party/include/glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
END_IGNORE_WARNINGS()
