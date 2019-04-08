// Copyright 2019, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include "warhol/graphics/common/render_command.h"

namespace warhol {

struct Mesh;

namespace opengl {

struct MeshHandles;
struct OpenGLRendererBackend;

bool OpenGLStageMesh(OpenGLRendererBackend*, Mesh*);

bool OpenGLRendererUploadMeshRange(OpenGLRendererBackend*, Mesh*,
                                   IndexRange vertex_range,
                                   IndexRange index_range);

void OpenGLUnstageMesh(OpenGLRendererBackend*, Mesh*);

// Actually deallocate the handles of a mesh from OpenGL.
void DeleteMeshHandles(MeshHandles* handles);

}  // namespace opengl
}  // namespace warhol
