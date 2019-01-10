// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

// This include file exists because MSVC does not make a good work of being able
// to define includes that don't generate warnings aka. -isystem in decent
// compilers (yeah... this was implemented in 2018... sue me).
//
// With this file I wrap over the includes instead of having to remember to put
// BEGIN/END ignore all the time.
//
// This file is meant to only bring the declarations (for including in headers).

#include "warhol/utils/macros.h"

BEGIN_IGNORE_WARNINGS()

#define GLM_FORCE_RADIANS

// Makes GLM perspective matrix to generate depth between 0.0 - 1.0, instead
// of OpenGL's -1.0 to 1,0.
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <third_party/include/glm/glm.hpp>

END_IGNORE_WARNINGS()
