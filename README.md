# Warhol

A simple game engine written from scratch.

The phylosophy of Warhol is "EaaL": Engine as a Library. The whole logic of the
game is your own and Warhol only provides common functionality. The way the
application is started, managed and loops is up to you, Warhol has no opinion.

This differs from engines like Unity in which you have to abide to their workflow,
specially using their own UI. Warhol provides a lot of tooling, but in the end
the UI and logic is up to you.

Is a game engine tooling meant for C++ programmers.

## Cross Platform

Currently Warhol supports Windows, Linux and MacOS. MacOS has the caveat of the
graphics API backend. Warhol itself is agnostic to the graphics API used to render,
by using the concepts of RenderBackends. Current only the OpenGL backend has
been fleshed out. This backend uses modern OpenGL (3.2+). If you cannot get a
modern OpenGL context, then you don't get to use Warhol for now :(

A Vulkan backend is planned, which would enable you to use it on MacOS with
MoltenVK.

## About YCM

I use YCM extensively, but every machine has it's own flags that differ a lot,
specially in cross-platform development. For this, I have modified the
`.ycm_extra_conf.py` YCM file to try to import a python file called
`ycm_extra_conf_local.py'. This file should implement a function called
`GetYCMLocalFlags` that should return a python array with the flags YCM style:

```
local_flags [
  "-I", "/some/path/to/includes",
  "-isystem", "/other/path/to/system/includes",
]

def GetYCMLocalFlags():
  return local_flags
```

This file is not tracked in versioning, so you must provide your own.
