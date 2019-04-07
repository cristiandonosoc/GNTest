# TODOs

## Graphics

- Stageable resources (Shaders, Meshes, Textures) currently don't track if
  they're loaded. They should so it's easier to detect leaking and implement
  RAII semantics.
- Define a type of an Index instead of relying always on uint32_t
