# Qx
<img align="left" src="https://i.imgur.com/TzdFQfb.png" width=25%>
Qx is a basic C++ library, intended to act as a direct extension to the Qt project, with facilities and a composition that are designed to feel familiar and follow in the spirit of the original framework.

This project makes use of the CMake build system generator for both compilation and consumption of the library.

It is based on Qt 6.

[![Dev Builds](https://github.com/oblivioncth/Qx/actions/workflows/build-project.yml/badge.svg?branch=dev)](https://github.com/oblivioncth/Qx/actions/workflows/build-project.yml)

## Documentation:
Detailed documentation of this library, facilitated by Doxygen, is available at: https://oblivioncth.github.io/Qx/

### Highlights:

- [Qx::ApplicationLogger](https://oblivioncth.github.io/Qx/classQx_1_1ApplicationLogger.html)
- [Qx::AsyncDownloadManager](https://oblivioncth.github.io/Qx/classQx_1_1AsyncDownloadManager.html)/[Qx::SyncDownloadManager](https://oblivioncth.github.io/Qx/classQx_1_1SyncDownloadManager.html)
- [Qx::Base85](https://oblivioncth.github.io/Qx/classQx_1_1Base85.html)
- [Qx::Cumulation< K, V >](https://oblivioncth.github.io/Qx/classQx_1_1Cumulation.html)
- [Qx::Table< T >](https://oblivioncth.github.io/Qx/classQx_1_1Table.html)/[Qx::DsvTable](https://oblivioncth.github.io/Qx/classQx_1_1DsvTable.html)
- [Qx::Error](https://oblivioncth.github.io/Qx/classQx_1_1Error.html)
- [Qx::GroupedProgressManager](https://oblivioncth.github.io/Qx/classQx_1_1GroupedProgressManager.html)
- [Qx::Json](https://oblivioncth.github.io/Qx/qx-json_8h.html)
- [Qx::SetOnce< T >](https://oblivioncth.github.io/Qx/classQx_1_1SetOnce.html)
- [Qx::TaskbarButton](https://oblivioncth.github.io/Qx/classQx_1_1TaskbarButton.html)
- [qx-common-io.h](https://oblivioncth.github.io/Qx/qx-common-io_8h.html)
- [qx-concepts.h](https://oblivioncth.github.io/Qx/qx-concepts_8h.html)
- [qx-system.h](https://oblivioncth.github.io/Qx/qx-system_8h.html)

## Disclaimer
**Use this library in your own projects with caution.**

This library began as personal library that I continue to maintain for use in my own Qt-based projects. I have decided to make it public in order to allow for full transparency in said projects and ensure that I'm abiding by open source license restrictions. It has evolved to be sensibly packaged and easily consumed by anyone, but since this library is largely intended for personal use there are currently no guarantees that are generally associated with production frameworks (i.e. stable ABI, minimal non-breaking changes, etc.)

**However**, I do attempt to keep my changes organized and reasonable, and in the process of releasing this repository I heavily cleaned up the codebase and committed to fully documenting its API, so using this library in other projects is feasible if you so desire.

The recommendation is to use the static version of the library in order to avoid ABI issues given it's adolescent nature.

If you do end up using this project, either through my other software or in your own, feel free to contribute, complain, point out bugs, or offer suggestions.

## Getting Started
Either grab the latest [release](https://github.com/oblivioncth/Qx/releases/) or [build the library from source](https://oblivioncth.github.io/Qx/index.html#autotoc_md3), and import using CMake.

Building from source is recommended as this library can easily be integrated as a dependency into your project using CMake's FetchContent. An example of this is demonstrated in the documentation.

Either way you'll then need to explore the [documentation](https://oblivioncth.github.io/Qx/index.html), which expands on acquiring, setting up, and using Qx.

### Summary

 - C++20
 - CMake 3.23.0

### Dependencies
- Qt6
- [OBCMake](https://github.com/oblivioncth/OBCmake) (build script support, fetched automatically)
- [Doxygen](https://www.doxygen.nl/)  (for documentation)

## Pre-built Releases/Artifacts

Releases and some workflows currently provide builds of Qx in various combinations of platforms and compilers. View the repository [Actions](https://github.com/oblivioncth/Qx/actions) or [Releases](https://github.com/oblivioncth/Qx/releases) to see examples.

For all builds, Qt was configured as follows (excluding defaults):

 - Release
 - Compiler
    - Windows: win32-msvc
    - Linux: linux-clang
 - Shared/Static Linkage
 - Modules: qtbase, qtimageformats, qtnetworkauth, qtsvg, qt5compat
 - Features: relocatable
 - -ssl (Linux) / -schannel (Windows)
