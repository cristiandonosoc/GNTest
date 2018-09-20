WARHOL_BASE=/mnt/c/Programming/projects/warhol

shaders: FORCE
	glslangValidator -V shaders/simple.vert -o out/simple.vert.spv
	glslangValidator -V shaders/simple.frag -o out/simple.frag.spv

win_shaders: FORCE
	glslangValidator.exe -I${WARHOL_BASE} -V shaders/simple.vert -o out/simple.vert.spv
	glslangValidator.exe -I${WARHOL_BASE} -V shaders/simple.frag -o out/simple.frag.spv

FORCE:
