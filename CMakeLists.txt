cmake_minimum_required (VERSION 3.15)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(ENKI_DEMOS "Build all demos" OFF)
option(ENKI_TESTS "Build tests" OFF)
option(ENKI_TRACY "Tracy profiler client for Enki" ON)

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

include(FetchContent)
FetchContent_Declare(tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy
    GIT_TAG        v0.6.3
    SOURCE_DIR     "${FETCHCONTENT_BASE_DIR}/tracy"
)
FetchContent_GetProperties(tracy)
if (NOT tracy_POPULATED)
    FetchContent_Populate(tracy)
    set(tracy_SOURCE_DIR "${FETCHCONTENT_BASE_DIR}/tracy")# PARENT_SCOPE)
endif ()

add_subdirectory ("src")

if (ENKI_TRACY)
	target_compile_definitions(Enki PUBLIC TRACY_ENABLE)
endif()

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