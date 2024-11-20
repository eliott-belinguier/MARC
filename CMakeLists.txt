cmake_minimum_required(VERSION 3.22)
project(RoverRun C)

set(CMAKE_C_STANDARD 99)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)