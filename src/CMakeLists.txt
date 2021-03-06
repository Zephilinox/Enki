﻿cmake_minimum_required (VERSION 3.15)

project(Enki)

set(Enki_HEADERS 
	"Enki/Networking/Packet.hpp"
	"Enki/Networking/RPC.hpp"
	"Enki/Networking/RPCManager.hpp"
	"Enki/Networking/Client.hpp"
	"Enki/Networking/ClientHost.hpp"
	"Enki/Networking/ClientStandard.hpp"
	"Enki/Networking/Server.hpp"
	"Enki/Networking/ServerHost.hpp"
	"Enki/Networking/ServerStandard.hpp"
	
	"Enki/Managers/NetworkManager.hpp"
	"Enki/Managers/TextureManager.hpp"
	"Enki/Managers/InputManager.hpp"
	"Enki/Managers/FontManager.hpp"
	"Enki/Managers/SoundManager.hpp"
	
	"Enki/Signals/Connection.hpp"
	"Enki/Signals/Disconnector.hpp"
	"Enki/Signals/Signal.hpp"
	"Enki/Signals/Slot.hpp"
	
	"Enki/Messages/Message.hpp"
	"Enki/Messages/MessageQueue.hpp"
	"Enki/Messages/MessageFunction.hpp"

	"Enki/GUI/IMGUI/imconfig.h"
	"Enki/GUI/IMGUI/imgui.h"
	"Enki/GUI/IMGUI/imgui_internal.h"
	"Enki/GUI/IMGUI/imgui_SFML.h"
	"Enki/GUI/IMGUI/imstb_rectpack.h"
	"Enki/GUI/IMGUI/imstb_truetype.h"
	"Enki/GUI/IMGUI/imstb_textedit.h"
	
	"Enki/GUI/Console.hpp"
	"Enki/GUI/ScenetreeGUI.hpp"

	"Enki/Input/Keys.hpp"
	"Enki/Input/Events.hpp"

	"Enki/Window/Window.hpp"
	"Enki/Window/WindowLog.hpp"
	"Enki/Window/WindowNone.hpp"

	"Enki/Renderer/Renderer.hpp"
	"Enki/Renderer/RendererLog.hpp"
	"Enki/Renderer/RendererNone.hpp"
	
	"Enki/Graphics/Font.hpp"
	"Enki/Graphics/FontLog.hpp"
	"Enki/Graphics/FontNone.hpp"
	"Enki/Graphics/Sprite.hpp"
	"Enki/Graphics/SpriteLog.hpp"
	"Enki/Graphics/SpriteNone.hpp"
	"Enki/Graphics/Text.hpp"
	"Enki/Graphics/TextLog.hpp"
	"Enki/Graphics/TextNone.hpp"
	"Enki/Graphics/Texture.hpp"
	"Enki/Graphics/TextureLog.hpp"
	"Enki/Graphics/TextureNone.hpp"
	"Enki/Graphics/Colour.hpp"
	"Enki/Graphics/Vector2.hpp"

	#todo: detect SFML is available
	"Enki/Input/SFML/KeyConversion.hpp"
	"Enki/Window/WindowSFML.hpp"
	"Enki/Renderer/RendererSFML.hpp"
	"Enki/Graphics/FontSFML.hpp"
	"Enki/Graphics/SpriteSFML.hpp"
	"Enki/Graphics/TextSFML.hpp"
	"Enki/Graphics/TextureSFML.hpp"

	"Enki/Timer.hpp"
	"Enki/TimerTrigger.hpp"
	"Enki/Scenetree.hpp"
	"Enki/Entity.hpp"
	"Enki/RNG.hpp"
	"Enki/Hash.hpp"
	"Enki/AnimatedSprite.hpp"
	"Enki/TimerLog.hpp"
	"Enki/MustMove.hpp"
)

set (Enki_SRC
	"Enki/Networking/Packet.cpp"
	"Enki/Networking/ClientHost.cpp"
	"Enki/Networking/ClientStandard.cpp"
	"Enki/Networking/ServerHost.cpp"
	"Enki/Networking/ServerStandard.cpp"
	"Enki/Networking/RPCManager.cpp"
	"Enki/Networking/Client.cpp"
	"Enki/Networking/Server.cpp"
	
	"Enki/Managers/NetworkManager.cpp"
	"Enki/Managers/TextureManager.cpp"
	"Enki/Managers/InputManager.cpp"
	"Enki/Managers/FontManager.cpp"
	"Enki/Managers/SoundManager.cpp"
	
	"Enki/Signals/Connection.cpp"
	
	"Enki/Messages/MessageQueue.cpp"

	"Enki/GUI/IMGUI/imgui.cpp"
	"Enki/GUI/IMGUI/imgui_demo.cpp"
	"Enki/GUI/IMGUI/imgui_draw.cpp"
	"Enki/GUI/IMGUI/imgui_SFML.cpp"
	"Enki/GUI/IMGUI/imgui_widgets.cpp"
	
	"Enki/GUI/Console.cpp"
	"Enki/GUI/ScenetreeGUI.cpp"

	"Enki/Input/Keys.cpp"
	
	"Enki/Window/WindowLog.cpp"
	"Enki/Window/WindowNone.cpp"
	
	"Enki/Renderer/RendererLog.cpp"
	"Enki/Renderer/RendererNone.cpp"
	
	"Enki/Graphics/FontLog.cpp"
	"Enki/Graphics/FontNone.cpp"
	"Enki/Graphics/SpriteLog.cpp"
	"Enki/Graphics/SpriteNone.cpp"
	"Enki/Graphics/TextureLog.cpp"
	"Enki/Graphics/TextureNone.cpp"
	"Enki/Graphics/TextLog.cpp"
	"Enki/Graphics/TextNone.cpp"

	#todo: detect SFML is available
	"Enki/Input/SFML/KeyConversion.cpp"
	"Enki/Window/WindowSFML.cpp"
	"Enki/Renderer/RendererSFML.cpp"
	"Enki/Graphics/FontSFML.cpp"
	"Enki/Graphics/SpriteSFML.cpp"
	"Enki/Graphics/TextSFML.cpp"
	"Enki/Graphics/TextureSFML.cpp"

	"Enki/Timer.cpp"
	"Enki/TimerTrigger.cpp"
	"Enki/Scenetree.cpp"
	"Enki/RNG.cpp"
	"Enki/AnimatedSprite.cpp"
	"Enki/TimerLog.cpp"
)

#include("../cmake/FilesystemSourceGroup.cmake")
#SOURCE_GROUP_BY_FOLDER(Enki)

include("../cmake/PreTargetSettings.cmake")
add_library(Enki
	${Enki_HEADERS}
	${Enki_SRC}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Enki FILES ${Enki_HEADERS} ${Enki_SRC})
set_property(TARGET Enki PROPERTY CMAKE_CXX_EXTENSIONS OFF)

include("../cmake/PostTargetSettings.cmake")

set_target_properties(
	Enki PROPERTIES
	#This is visual studio specific, can't remember what it does
	VS_DEBUGGER_WORKING_DIRECTORY		"${CMAKE_SOURCE_DIR}"
	#the .exe goes in this folder
	RUNTIME_OUTPUT_DIRECTORY			"${CMAKE_SOURCE_DIR}/bin/Enki/${CONFIGURATION}"
	#for shared libraries, should this be different to static?
	LIBRARY_OUTPUT_DIRECTORY			"${CMAKE_SOURCE_DIR}/bin/Enki/lib"
	LIBRARY_OUTPUT_DIRECTORY_DEBUG		"${CMAKE_SOURCE_DIR}/bin/Enki/lib"
	LIBRARY_OUTPUT_DIRECTORY_RELEASE	"${CMAKE_SOURCE_DIR}/bin/Enki/lib"
	#for static libraries
	ARCHIVE_OUTPUT_DIRECTORY			"${CMAKE_SOURCE_DIR}/bin/Enki/lib"
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG		"${CMAKE_SOURCE_DIR}/bin/Enki/lib"
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE	"${CMAKE_SOURCE_DIR}/bin/Enki/lib"
	#for debug lib suffix
	DEBUG_POSTFIX						"-d"
)

if (UNIX)
	set(UNIX_LIBS pthread)
endif()

target_link_libraries(Enki PUBLIC
	sfml-graphics
	sfml-audio
	spdlog
	enet
	${UNIX_LIBS}
)

target_include_directories(Enki PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
)

#couldn't figure out a better way to do this, don't really want to use install targets
#remove old headers
file(REMOVE_RECURSE "${CMAKE_SOURCE_DIR}/bin/Enki/include")

#copy current ones
file(COPY "${CMAKE_SOURCE_DIR}/src/Enki"
    DESTINATION "${CMAKE_SOURCE_DIR}/bin/Enki/include"
    FILES_MATCHING PATTERN "*.hpp"
)
