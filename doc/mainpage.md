Qx {#mainpage}
==============
Qx is a basic C++ library built on top of the ubiquitous Qt framework, created with the intention of extending its existing functionality while following the same paradigm of its design.

The library primarily consists data structures and routines that facilitate a variety of common, or otherwise reoccurring programmatic tasks, largely providing convenience, flexibility, and syntactic efficiency to developers when writing applications.

Requirements
------------

 - A C++20 capable compiler
 - Qt 6.2.x
 - CMake 3.21.1 or greater
 - A Qt compatible platform (only tested on Windows and Debian Linux!)

Packaging
----------
Qx is provided as a CMake package containing several component libraries that roughly follow the scope and grouping of Qt modules, with Core being the primary component.

See the [Components Index](modules.html) for a list of available components (**NOTE**: Some of these are platform dependent).

When importing the library package with CMake it is recommended to only include the components you need for a particular application in order to minimize the size of your build.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Example Qx Import
find_package(Qx REQUIRED COMPONENTS Core Network)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
An imported component that depends on other Qx components will always cause said components to automatically be imported, even if they are not explicitly specified. Likewise, targets that link to interdependent components will automatically be linked to dependency components as necessary.

Getting Started
---------------
1) Download the latest [Release](https://github.com/oblivioncth/Qx/releases).

2) Place the package somewhere CMake can find it
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Add to a default search path or...
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} path\to\Qx_package)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

3) Import the package
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# When no components are specified, all available components will be imported
find_package(Qx 0.1) # Or whichever version
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

4)	Link to the required components where necessary
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
target_link_libraries(example_ui_app PUBLIC Qx::Core Qx::Widgets)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

5) Include the corresponding headers in your code
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "qx/core.h"
#include "qx/widgets.h"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

6) Review the rest of the documentation. A good place to start is the [Core](#qx-core) component.

**NOTE:** 
It is possible to include portions of the Qx API with finer granularity by only referencing the exact headers you need in accordance with the include structure shown in the [File Index](files.html), but it is generally recommended to simply include the main header for each component you use as shown above.

Building From Source
--------------------
The latest source is available in the Master branch of https://github.com/oblivioncth/Qx.

The requirements for building from Git are the same as for using Qx, with the obvious exception that Doxygen is also needed to build the documentation.

If newer to working with Qt, it is easiest to build from within Qt creator as it handles a large portion of environment setup, including adding Qt to CMake's package search list, automatically. Simply make sure that a kit is configured in Qt Creator that uses a compatible version of Qt, open the CMakeLists.txt file as a project, and build with the desired configuration.

The CMake project is designed to be used with multi-configuration generators such as Visual Studio or Ninja Multi-Config, and may require some tweaking to work with single configuration generators.

#### CMake Targets:

 - all - Builds all Qx components (the entire library)
 - install - Installs the build output of the provided configuration (--config X) into the *out* directory
 - docs - Builds the Qx documentation

#### Package
By default, the CMakeLists project configures CPack to create an artifact ZIP containing the binaries for Debug and Release configurations, as well as documentation.

To properly create the full package, perform the following steps inside the source directory:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Build the Debug libraries
cmake.exe --build <path/to/build/dir> --target all --config Debug

# Build the Release libraries
cmake.exe --build <path/to/build/dir> --target all --config Release

# Build the documentation
cmake.exe --build <path/to/build/dir> --target docs --config Release

# Install Debug/Release libraries and documentation
cmake.exe --build <path/to/build/dir> --target install --config Debug
cmake.exe --build <path/to/build/dir> --target install --config Release

# Create the output package
cpack.exe -C Debug;Release
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**NOTE:**
For the time being, Qx is only configured to be built as a static library, which means that the Qt binaries you use to make Qx must have also been built using static linking. No pre-built distributions of Qt are provided that use this configuration and so you must built Qt yourself in this manner.

See https://doc.qt.io/qt-6/windows-building.html for general information and be sure to pass the **-static** option to the *configure* script.

If you get stuck building Qt statically (as there can be some gotchyas), feel free to open an issue under the project's Github repository for assistance.
