# -*- coding: utf-8 -*-
# Copyright 2019, Cristián Donoso.
# This code has a BSD license. See LICENSE.

import os
import sys

VERT_SHADER = 1
FRAG_SHADER = 2

def ParseShader(base_dir, shader_name, shader_type):
    if shader_type == VERT_SHADER:
        shader_filename = shader_name + ".vert"
    elif shader_type == FRAG_SHADER:
        shader_filename = shader_name + ".frag"
    else:
        raise Exception("Invalid shader type: {}".format(shader_type))

    path = os.path.join(base_dir, shader_filename)
    lines = [line.strip("\n\t\r ") for line in open(path)]

    # Parse the shader for keywords.
    includes = []
    blocks_data = []
    for i, line in enumerate(lines):
        if line.startswith("//#INCLUDE"):
            source = ParseInclude(base_dir, line)
            includes.append(dict(line=i, source=source))
        elif line.startswith("UNIFORM_BLOCK"):
            if block_data:
                raise Exception("Uniform block already found.")
            block_data = ParseUniformBlock(lines, i)

    # Join the shader back in.
    new_lines = []
    for i, line in enumerate(lines):
        # See if we need to append a include file.
        include_added = False
        for include in includes:
            if include["line"] == i:
                new_lines.append(include["source"])
                include_added = True
                break

        # If an included was not added, we simply append the line.
        if include_added:
            continue
        new_lines.append(line)

    final_source = "\n".join(new_lines)
    print final_source

def ParseInclude(base_dir, line):
    tokens = line.split(" ")
    if (len(token) != 2):
        raise Exception("Too many tokens (Expected 2): {}".format(line))

    if (tokens[0] != "//#INCLUDE"):
        raise Exception("Invalid include line: {}".format(line))

    include_filename = token[1].trim("\"")
    include_path = os.path.join(base_dir, include_path)
    with open(include_path) as f:
        return f.read()

# Is the current line number where the unifrom block was found.
def ParseUniformBlock(lines, i):




if __name__ == "__main__":
    shader_name = sys.argv[1]
    base_dir = sys.argv[2]
    ParseShader(base_dir, shader_name, VERT_SHADER)
