# what2watch - a Movie Recommendation App

what2watch is a simple toy app built to help me gain experience in writing high-level C
and as an excuse to write data serialization and layout libraries in C

It uses NanoVG for graphics and the Bullet bt3gui library for cross-platform windowing and Glew for OpenGL.

Thanks to syoyo's [nanocanvas](https://github.com/syoyo/nanocanvas) which was used as a template.

## Platform

* Mac OS X
* Linux
* Windows
* iOS/Android (TODO)

## Requirements

* OpenGL2
  * GLESv2
* premake5

## Build

### Linux and MacOSX

    $ premake5 gmake
    $ cd build
    $ make

### Windows

Visual Studio 2015 is required to build an example.

    > premake5.exe vs2015
    > cd build

Open solution file and build it with Visual Studio 2015.

Visual Studio 2013 may work. To compile with Visual Studio 2013 use the following premake flag.

    > premake5.exe vs2013

## License

NanoCanvas example code is licensed under MIT license.
NanoCanvas uses third party libraries. See `LICENSES` file for more details.
