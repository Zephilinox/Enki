﻿cmake_minimum_required (VERSION 3.15)

target_include_directories(Enki PUBLIC "doctest/")
target_include_directories(Enki PUBLIC "doctest/parts/")

add_subdirectory ("enet")
add_subdirectory ("spdlog")

target_include_directories(Enki PUBLIC "enetpp/")
target_include_directories(Enki PUBLIC "enetpp/include/")
target_include_directories(Enki PUBLIC "enetpp/include/enetpp/")