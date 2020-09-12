cmake_minimum_required (VERSION 3.15)
include(FetchContent)

set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/third_party")

message(STATUS "############ Fetching fmt")
FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG        7.0.3
    GIT_SHALLOW    TRUE
    CMAKE_ARGS
        -DFMT_DOC=OFF
        -DFMT_INSTALL=OFF
        -DFMT_TEST=OFF
)
FetchContent_MakeAvailable(fmt)
set_target_properties(fmt PROPERTIES FOLDER third_party)
message(STATUS "############ Done")

message(STATUS "############ Fetching spdlog")
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG        v1.8.0
    GIT_SHALLOW    TRUE
)
if (NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
endif ()
add_library(spdlog INTERFACE)
target_include_directories(spdlog SYSTEM INTERFACE "${spdlog_SOURCE_DIR}/include")
target_compile_definitions(spdlog INTERFACE "SPDLOG_FMT_EXTERNAL")
target_link_libraries(spdlog INTERFACE fmt::fmt-header-only)
message(STATUS "############ Done")

message(STATUS "############ Fetching doctest")
FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/onqtam/doctest
    GIT_TAG        2.4.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(doctest)
set_target_properties(doctest_with_main PROPERTIES FOLDER third_party)
message(STATUS "############ Done")

if (ENKI_TRACY)
    message(STATUS "############ Fetching Tracy")
    include(FetchContent)
    FetchContent_Declare(tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy
        GIT_TAG        v0.7.1
        SOURCE_DIR     "${FETCHCONTENT_BASE_DIR}/tracy"
    )
    FetchContent_GetProperties(tracy)
    if (NOT tracy_POPULATED)
        FetchContent_Populate(tracy)
        set(tracy_SOURCE_DIR "${FETCHCONTENT_BASE_DIR}/tracy" PARENT_SCOPE)
    endif()
    message(STATUS "############ Done")
endif()

message(STATUS "############ Adding enet")
add_subdirectory ("enet")
set_target_properties(enet PROPERTIES FOLDER third_party)
message(STATUS "############ Done")