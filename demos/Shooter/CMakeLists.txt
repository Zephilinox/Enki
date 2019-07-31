cmake_minimum_required (VERSION 3.15)

project(Shooter)

add_executable(Shooter
"main.cpp"

"src/Managers/InputManager.hpp"
"src/Managers/InputManager.cpp"
"src/Managers/MapManager.hpp"
"src/Managers/MapManager.cpp"

"src/CustomData.hpp"

"src/Game.hpp"
"src/Game.cpp"

"src/Player.hpp"
"src/Player.cpp"

"src/Floor.hpp"
"src/Floor.cpp"

"src/Wall.hpp"
"src/Wall.cpp"

"src/HealthSpawner.hpp"
"src/HealthSpawner.cpp"
)

include("${EnkiEngine_SOURCE_DIR}/cmake/Settings.cmake")

set_target_properties(
	Shooter PROPERTIES
	#This is visual studio specific, can't remember what it does
	VS_DEBUGGER_WORKING_DIRECTORY "${EnkiEngine_SOURCE_DIR}/demos/Shooter"
	#the .exe goes in this folder
	RUNTIME_OUTPUT_DIRECTORY "${EnkiEngine_SOURCE_DIR}/bin/Shooter/${CONFIGURATION}"
	#not sure, library files don't seem to go here'
	LIBRARY_OUTPUT_DIRECTORY "${EnkiEngine_SOURCE_DIR}/bin/Shooter/${CONFIGURATION}"
)

target_link_libraries(Shooter Enki)

target_include_directories(Shooter PRIVATE ${EnkiEngine_SOURCE_DIR}/demos/Shooter/src/)

#remove old resources
file(REMOVE_RECURSE "${EnkiEngine_SOURCE_DIR}/bin/Shooter/Debug/resources/")
file(REMOVE_RECURSE "${EnkiEngine_SOURCE_DIR}/bin/Shooter/Release/resources/")

#copy current ones
file(COPY "${EnkiEngine_SOURCE_DIR}/demos/Shooter/resources/"
    DESTINATION "${EnkiEngine_SOURCE_DIR}/bin/Shooter/Debug/resources/"
)

file(COPY "${EnkiEngine_SOURCE_DIR}/demos/Shooter/resources/"
	DESTINATION "${EnkiEngine_SOURCE_DIR}/bin/Shooter/Release/resources/"
)