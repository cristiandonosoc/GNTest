WARHOL_BASE=/mnt/c/Programming/projects/warhol


make:
	ninja -C out opengl_example

test: FORCE
	ninja -C out tests
	out/tests

win_make:
	win_ninja -C out opengl_example


shaders: FORCE
	glslangValidator -V shaders/simple.vert -o out/simple.vert.spv
	glslangValidator -V shaders/simple.frag -o out/simple.frag.spv

win_shaders: FORCE
	glslangValidator.exe -I${WARHOL_BASE} -V shaders/simple.vert -o out/simple.vert.spv
	glslangValidator.exe -I${WARHOL_BASE} -V shaders/simple.frag -o out/simple.frag.spv

FORCE:
