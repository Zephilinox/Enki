#https://stackoverflow.com/questions/44960715/how-to-enable-stdc17-in-vs2017-with-cmake
if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
		set(CXX_STANDARD_ latest)
	endif()
endif()