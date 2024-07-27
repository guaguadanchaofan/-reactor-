cmake_minimum_required(VERSION 3.27.4)


set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_BUILD_TYPE "Debug")
aux_source_directory(. SRC_LIST)

add_library(mymuduo SHARED SRC_LIST)
