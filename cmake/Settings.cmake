set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
get_property(CXX_STANDARD_ TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD)
message(STATUS "CXX_STANDARD for ${PROJECT_NAME} is C++${CXX_STANDARD_}")

#https://stackoverflow.com/questions/2368811/how-to-set-warning-level-in-cmake
if (MSVC) 
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
	
	#disable for now, warnings as errors
	if (CMAKE_CXX_FLAGS MATCHES "/WX")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
	endif()
endif()
