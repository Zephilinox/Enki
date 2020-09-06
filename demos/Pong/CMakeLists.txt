cmake_minimum_required (VERSION 3.15)

project(Pong)

include("${EnkiEngine_SOURCE_DIR}/cmake/PreTargetSettings.cmake")

add_executable(Pong
"src/main.cpp"

"src/Ball.hpp"
"src/Ball.cpp"

"src/Collision.hpp"
"src/Collision.cpp"

"src/Game.hpp"
"src/Game.cpp"

"src/Paddle.hpp"
"src/Paddle.cpp"

"src/PlayerText.hpp"
"src/PlayerText.cpp"

"src/Score.hpp"
"src/Score.cpp"
)

get_target_property(Pong_SOURCES Pong SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${Pong_SOURCES})

include("${EnkiEngine_SOURCE_DIR}/cmake/PostTargetSettings.cmake")

set_target_properties(
	Pong PROPERTIES
	#This is visual studio specific, can't remember what it does
	VS_DEBUGGER_WORKING_DIRECTORY "${EnkiEngine_SOURCE_DIR}/demos/Pong/"
	#the .exe goes in this folder
	RUNTIME_OUTPUT_DIRECTORY "${EnkiEngine_SOURCE_DIR}/bin/Pong/${CONFIGURATION}"
	#not sure, library files don't seem to go here'
	LIBRARY_OUTPUT_DIRECTORY "${EnkiEngine_SOURCE_DIR}/bin/Pong/${CONFIGURATION}"
)

target_link_libraries(Pong Enki)

include_directories(${EnkiEngine_SOURCE_DIR}/src "src")

set_target_properties(Pong PROPERTIES FOLDER demos)

#remove old resources
file(REMOVE_RECURSE "${EnkiEngine_SOURCE_DIR}/bin/Pong/Debug/resources/")
file(REMOVE_RECURSE "${EnkiEngine_SOURCE_DIR}/bin/Pong/Release/resources/")

#copy current ones
file(COPY "${EnkiEngine_SOURCE_DIR}/demos/Pong/resources/"
    DESTINATION "${EnkiEngine_SOURCE_DIR}/bin/Pong/Debug/resources/"
)

file(COPY "${EnkiEngine_SOURCE_DIR}/demos/Pong/resources/"
    DESTINATION "${EnkiEngine_SOURCE_DIR}/bin/Pong/Release/resources/"
)