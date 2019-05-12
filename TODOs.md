# TODOs

## List

- List should be joinable (move one into another).
- Nodes should be extractable/movable.
- Currently Lists own their own memory, for ease of API, but eventually they
- should have exclusive externally owned memory sections.

## Graphics

- There should be a clear action, instead of doing it by default on New frame
  (like it's the case with the OpenGL render backend).
- Shader loading is now in common, so it has knowhow about opengl and vulkan.
  This should be moved into the backend.
- Path now are relative and hardcoded to a name (*especially* shaders).
  There should be a path object that holds where each resource is.
- Stageable resources (Shaders, Meshes, Textures) currently don't track if
  they're loaded. They should so it's easier to detect leaking and implement
  RAII semantics.
- Define a type of an Indices instead of relying always on uint32_t

## OpenGL

- Add some uniform loading testing so that we don't get annoying surprises down
  the line.
