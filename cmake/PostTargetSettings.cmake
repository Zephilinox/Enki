if (NOT CXX_STANDARD_ STREQUAL latest)
	set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
	set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
	get_property(CXX_STANDARD_ TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD)
endif()

message(STATUS "CXX_STANDARD for ${PROJECT_NAME} is C++${CXX_STANDARD_}")

set_property(TARGET ${PROJECT_NAME} PROPERTY CMAKE_CXX_EXTENSIONS OFF)

#https://stackoverflow.com/questions/2368811/how-to-set-warning-level-in-cmake
if (MSVC) 
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
	
	#if (CMAKE_CXX_FLAGS MATCHES "/W4")
	#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
	#endif()

	target_compile_options(${PROJECT_NAME} PUBLIC
		"/permissive-"
		"/Zc:__cplusplus"
		"/experimental:external"
		"/external:W0"
		"/external:anglebrackets"
	)
endif()

if (WIN32)
	target_compile_definitions(${PROJECT_NAME} PUBLIC
		NOMINMAX
		WIN32_LEAN_AND_MEAN
	)
endif()