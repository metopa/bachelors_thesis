cmake_minimum_required(VERSION 3.6)
project(NumDB)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(3rdparty/murmurhash2functor/lib)
add_subdirectory(test/unit)

include_directories(include)
include_directories(3rdparty/include)
include_directories(3rdparty/murmurhash2functor/include)

set(NUMBD_FILES)
#add_library(numdb ${NUMDB_FILES})
add_executable(main src/main.cpp)

add_executable(bench test/benchmark/bench.cpp)
target_link_libraries(bench benchmark pthread)

