cmake_minimum_required (VERSION 3.12.4)

target_include_directories(Enki PUBLIC "doctest/")
target_include_directories(Enki PUBLIC "doctest/parts/")

add_subdirectory ("enet")
set_target_properties(enet PROPERTIES FOLDER CMakePredefinedTargets)

add_subdirectory ("spdlog")

target_include_directories(Enki PUBLIC "enetpp/")
target_include_directories(Enki PUBLIC "enetpp/include/")

target_include_directories(Enki PUBLIC "pcg/")
target_include_directories(Enki PUBLIC "pcg/include/")