cmake_minimum_required (VERSION 3.15)

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
	
	"Enki/Timer.hpp"
	"Enki/TimerTrigger.hpp"
	"Enki/Scenetree.hpp"
	"Enki/Entity.hpp"
	"Enki/GameData.hpp"
	"Enki/Renderer.hpp"
	"Enki/RNG.hpp"
	"Enki/Hash.hpp"
	"Enki/AnimatedSprite.hpp"
	"Enki/TimerLog.hpp"
	"Enki/NonCopyable.hpp"
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
	
	"Enki/Timer.cpp"
	"Enki/TimerTrigger.cpp"
	"Enki/Scenetree.cpp"
	"Enki/Renderer.cpp"
	"Enki/RNG.cpp"
	"Enki/AnimatedSprite.cpp"
	"Enki/TimerLog.cpp"
)

include("../cmake/FilesystemSourceGroup.cmake")
SOURCE_GROUP_BY_FOLDER(Enki)

include("../cmake/PreTargetSettings.cmake")
add_library (Enki
	${Enki_HEADERS}
	${Enki_SRC}
)

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

target_link_libraries(Enki
	sfml-graphics
	sfml-audio
	spdlog
	enet
	${UNIX_LIBS}
)

target_include_directories(Enki PUBLIC
	"${CMAKE_SOURCE_DIR}/src/"
)

#couldn't figure out a better way to do this, don't really want to use install targets
#remove old headers
file(REMOVE_RECURSE "${CMAKE_SOURCE_DIR}/bin/Enki/include")

#copy current ones
file(COPY "${CMAKE_SOURCE_DIR}/src/Enki"
    DESTINATION "${CMAKE_SOURCE_DIR}/bin/Enki/include"
    FILES_MATCHING PATTERN "*.hpp"
)
