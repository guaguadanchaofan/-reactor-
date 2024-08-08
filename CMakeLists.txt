cmake_minimum_required(VERSION 3.22.1)
project(mynet)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_BUILD_TYPE "Debug")

aux_source_directory(. SRC_LIST)



add_library(mynet SHARED ${SRC_LIST})

add_executable(echo EchoServer.cc)

target_link_libraries(echo mynet pthread)
