# TODOs

- Correctly handle minimization on all platforms.
- Instead of doing one-off copy command buffers, there should be a
  pre allocated one for this purpose. This is what VkNeo does.
- Generate mipmaps in the asset pipeline and simply load them
  (stb_image_resize).
- Remove utils/limits defined class Limits, just use the stdint.h ones.
