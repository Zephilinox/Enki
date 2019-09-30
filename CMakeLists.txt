cmake_minimum_required (VERSION 3.12.4)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(DEMOS "Build all demos" ON)
option(TESTS "Build tests" ON)

set (CMAKE_MODULE_PATH "${EnkiEngine_SOURCE_DIR}/cmake")

#Includes library and demos and stuff
project ("EnkiEngine")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Code Coverage Configuration
add_library(coverage_config INTERFACE)

option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	message(STATUS "CODE_COVERAGE ON")
  # Add required flags (GCC & LLVM/Clang)
  target_compile_options(coverage_config INTERFACE
    -O0        # no optimization
    -g         # generate debug info
    --coverage # sets all required flags
  )
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
    target_link_options(coverage_config INTERFACE --coverage)
  else()
    target_link_libraries(coverage_config INTERFACE --coverage)
  endif()
endif()

if (WIN32)
	set(SFML_STATIC_LIBRARIES TRUE)
	if ("${SFML_DIR}" STREQUAL "SFML_DIR-NOTFOUND")
		set(SFML_DIR "${CMAKE_SOURCE_DIR}/../SFML-2.5.1/lib/cmake/SFML")
		message(STATUS "Make sure SFML is installed and SFML_DIR is set")
		message(STATUS "SFML_DIR set to " ${SFML_DIR})
	endif()
else()
	#SFML isn't built statically for distros or something?
	message(STATUS "Make sure SFML is installed")
	set(SFML_STATIC_LIBRARIES FALSE)
	message(STATUS "SFML_DIR is set to " ${SFML_DIR})
endif()

find_package(SFML 2.5 COMPONENTS graphics audio REQUIRED)

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
