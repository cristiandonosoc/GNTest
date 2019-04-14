WARHOL_BASE=/mnt/c/Programming/projects/warhol


make: test FORCE
	ninja -C out experiments/new_api

test: FORCE
	ninja -C out tests
	out/tests

win: FORCE
	ninja.exe -C out experiments/new_api

shaders: FORCE
	glslangValidator -V assets/shaders/vulkan/demo.vert -o out/assets/shaders/vulkan/demo.vert.spv
	glslangValidator -V assets/shaders/vulkan/demo.frag -o out/assets/shaders/vulkan/demo.frag.spv

win_shaders: FORCE
	glslangValidator.exe -I${WARHOL_BASE} -V shaders/simple.vert -o out/simple.vert.spv
	glslangValidator.exe -I${WARHOL_BASE} -V shaders/simple.frag -o out/simple.frag.spv

FORCE:
