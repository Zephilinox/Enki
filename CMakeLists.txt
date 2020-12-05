cmake_minimum_required (VERSION 3.15)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(ENKI_BUILD_DEMOS "Build all demos" OFF)
option(ENKI_BUILD_TESTS "Build tests" OFF)

set(CMAKE_MODULE_PATH "${EnkiEngine_SOURCE_DIR}/cmake")

#Includes library and demos and stuff
project("EnkiEngine")

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/MP>")

if (WIN32)
	message(STATUS "SFML - Static Libraries")
	set(SFML_STATIC_LIBRARIES TRUE)
else()
	message(STATUS "SFML - Dynamic Libraries")
	set(SFML_STATIC_LIBRARIES FALSE)
endif()

find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)

add_subdirectory ("src")
set_property(GLOBAL PROPERTY VS_STARTUP_PROJECT Enki)

#all the demo projects
if (ENKI_BUILD_DEMOS)
	message(STATUS "Building Demos")
	add_subdirectory ("demos")
endif()

#the enkinet library tests
if (ENKI_BUILD_TESTS)
	message(STATUS "Building Tests")
	add_subdirectory("tests")
endif()

add_subdirectory ("third_party")

add_custom_target(EnkiEngine SOURCES
	"CMakeLists.txt"
	".clang-format"
	".gitignore"
	"LICENSE"
	"README.md"
	"cmake/FilesystemSourceGroup.cmake"
	"cmake/PostTargetSettings.cmake"
	"cmake/PreTargetSettings.cmake")
get_target_property(EnkiEngine_SOURCES EnkiEngine SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EnkiEngine_SOURCES})