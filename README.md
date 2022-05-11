# Qx
<img align="left" src="https://i.imgur.com/TzdFQfb.png" width=25%>
Qx is a basic C++ library, intended to act as a direct extension to the Qt project, with facilities and a composition that are designed to feel familiar and follow in the spirit of the original framework.

This project makes use of the CMake build system generator for both compilation and consumption of the library.

It is based on Qt 6.

[![Dev Builds](https://github.com/oblivioncth/Qx/actions/workflows/push-reaction.yml/badge.svg)](https://github.com/oblivioncth/Qx/actions/workflows/push-reaction.yml)

## Documentation:
Detailed documentation of this library, facilitated by Doxygen, is available at: https://oblivioncth.github.io/Qx/

## Disclaimer
**I cannot fully recommend using this library in your own projects.**

This library is essentially a personal library that I maintain for use in my own Qt-based projects. I have decided to make it public in order to allow for full transparency in said projects and ensure that I'm abiding by open source license restrictions. Since this library is largely intended for personal use, there are no guarantees that are generally associated with production frameworks (i.e. stable ABI, minimal non-breaking changes, etc.)

**However**, I do attempt to keep my changes organized and reasonable, and in the process of releasing this repository I heavily cleaned up the codebase and fully documented its API, so using this library in other projects is feasible if you so desire.

At the moment, this library is only configured to be built as a static lib, so ABI changes are not a real issue for the time being.

If you do end up using this project, either through my other software or in your own, feel free to contribute, complain, point out bugs, or offer suggestions.

## Getting Started
Either grab the latest [release](https://github.com/oblivioncth/Qx/releases/) or [build the library from source](https://oblivioncth.github.io/Qx/index.html#autotoc_md3), and import using CMake.

Building from source is recommended as this library can easily be integrated as a dependency into your project using CMake's FetchContent. An example of this is demonstrated in the documentation.

Either way you'll then need to explore the [documentation](https://oblivioncth.github.io/Qx/index.html), which expands on acquiring, setting up, and using Qx.

## Pre-built Releases/Artifacts

Releases and some workflows currently provide builds of Qx in the following configurations:

1) - Windows (windows-latest)
    - MSVC (latest)
    - Debug & Release
    - Static Linkage
    - Statically Linked Qt
>>
2) - Windows (windows-latest)
    - MSVC (latest)
	- Debug & Release
	- Static Linkage
	- Dynamically Linked Qt

For all builds, Qt was configured as follows (excluding defaults):

 - Release
 - Shared/Static Linkage
 - Modules: qtbase, qtimageformats, qtnetworkauth, qtsvg
 - Features: relocatable
 - -ssl (Linux) / -schannel (Windows)
