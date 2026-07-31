cmake_minimum_required(VERSION 3.20)

project(RC4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(tinyAES STATIC aes.c aes.hpp)
add_library(tinyAES::tinyAES ALIAS tinyAES)

target_include_directories(tinyAES PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
