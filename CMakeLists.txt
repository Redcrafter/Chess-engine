cmake_minimum_required(VERSION 3.9)
project(Chess C CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenMP REQUIRED)

file(GLOB_RECURSE Common_sources "./src/*.cpp" "./src/*.h")

add_executable(Chess ${Common_sources})

target_compile_options(Chess PUBLIC "-march=native")
# target_compile_options(Chess PUBLIC "-g")
target_compile_options(Chess PUBLIC "-O3")

target_include_directories(Chess PRIVATE "./src")
target_link_libraries(Chess PUBLIC OpenMP::OpenMP_CXX)
