WARHOL_BASE=/mnt/c/Programming/projects/warhol

win_shaders: FORCE
	glslangValidator.exe -I${WARHOL_BASE} -V shaders/simple.vert -o out/simple.vert.spv
	glslangValidator.exe -I${WARHOL_BASE} -V shaders/simple.frag -o out/simple.frag.spv

FORCE:
