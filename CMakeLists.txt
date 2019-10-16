cmake_minimum_required (VERSION 2.6)
set (CMAKE_CXX_STANDARD 17)

project (Chess)

include_directories("./src")

file(GLOB Common_sources "./src/**/*.cpp")

add_executable(Chess ${Common_sources} "./src/Main.cpp") 
