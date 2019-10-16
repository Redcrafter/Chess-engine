cmake_minimum_required (VERSION 2.6)
project (Chess C CXX)

set (CMAKE_CXX_STANDARD 17)
add_compile_options(-march=native)

include_directories("./src")
file(GLOB Common_sources "./src/**/*.cpp")

find_package(Threads REQUIRED)
add_executable(Chess ${Common_sources} "./src/Main.cpp") 
target_link_libraries(Chess ${CMAKE_THREAD_LIBS_INIT})
