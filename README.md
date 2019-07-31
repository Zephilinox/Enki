# Enki
Enki is a 2D Game Engine in C++17, and a continuation of EnkiNet, which was my dissertation project for a networked scenegraph.

## Instructions
### Make sure SFML is installed
If on windows, modify SFML_DIR in /CMakeLists.txt to point to the SFML install (not source code) (2.5.1, 32-bit)
If on linux, make sure to install libsfml-dev

### Generate solution with CMake

If on linux, choose between GCC or Clang

````
export CC=/usr/bin/gcc
export CXX=/usr/bin/gxx
````

or

````
export CC=/usr/bin/clang-8
export CXX=/usr/bin/clang++-8
````

make an empty folder, in this case I've called it build (which is ignored by git), then:


````
cd build
cmake ..
````

if generating a visual studio solution, remember to change the start-up project.

If CMake complains about not being able to find SFML when it's already installed, either run `cmake ..` again or make sure you've modified the SFML_DIR path and it's pointing to the correct SFML version (2.5.1, 32-bit)

Alternatively you may be using a 64-bit toolchain when SFML is 32-bit, for instance VS 2019. Run

````
cd build
cmake -G "Visual Studio 2019" -A Win32 ..
````

before `cmake ..`. You may need to clear out the build folder before/after doing so.

### Build with CMake
Either open the resulting IDE project file depending on your generator (like a visual studio solution), or run

````
cd build
cmake --build .
````

to build it directly using your chosen toolchain

### Demos and Tests

`cd build`

then

`cmake -D DEMOS=ON ..`

or

`cmake -D TESTS=ON ..`

or

`cmake -D DEMOS=ON -D TESTS=ON ..`

### Logging

Enki uses SPDLOG for logging. In order to disable or limit which levels of logging are outputted, as well as change the output location, you need to create a logger with the name "Enki" and modify it accordingly.

In the sample below the logger is modified after being constructed by Enki. Enki will construct a logger if one doesn't already exist when the Scenegraph, RPCManager, or NetworkManager are constructed.

````
auto enki_logger = spdlog::get("Enki");
enki_logger->set_level(spdlog::level::err);
````
