cmake_minimum_required(VERSION 3.6)
project(NumDB)

set(CMAKE_CXX_STANDARD 14)

set(NUMBD_FILES)
#add_library(numdb ${NUMDB_FILES})
include_directories(include)
add_executable(main src/main.cpp)
