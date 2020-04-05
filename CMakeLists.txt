cmake_minimum_required (VERSION 3.15)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(DEMOS "Build all demos" ON)
option(TESTS "Build tests" ON)

set (CMAKE_MODULE_PATH "${EnkiEngine_SOURCE_DIR}/cmake")

#Includes library and demos and stuff
project ("EnkiEngine")

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/MP>")

find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)

if (WIN32)
message(STATUS "SFML - Static Libraries")
	set(SFML_STATIC_LIBRARIES TRUE)
else()
	message(STATUS "SFML - Dynamic Libraries")
	set(SFML_STATIC_LIBRARIES FALSE)
endif()

add_subdirectory ("src")
set_property(GLOBAL PROPERTY VS_STARTUP_PROJECT Enki)

#all the demo projects
if (DEMOS)
	message(STATUS "Building Demos")
	add_subdirectory ("demos")
endif()

#the enkinet library tests
if (TESTS)
	message(STATUS "Building Tests")
	add_subdirectory("tests")
endif()

#exclude from all does something I'm pretty sure
add_subdirectory ("third_party" EXCLUDE_FROM_ALL)
