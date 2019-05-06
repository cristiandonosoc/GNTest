# TODOs

## List

- List should be joinable (move one into another).
- Nodes should be extractable/movable.

## Graphics

- Shader loading is now in common, so it has knowhow about opengl and vulkan.
  This should be moved into the backend.
- Path now are relative and hardcoded to a name (*especially* shaders).
  There should be a path object that holds where each resource is.
- Stageable resources (Shaders, Meshes, Textures) currently don't track if
  they're loaded. They should so it's easier to detect leaking and implement
  RAII semantics.
- Define a type of an Indices instead of relying always on uint32_t
