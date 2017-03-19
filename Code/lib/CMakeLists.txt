cmake_minimum_required(VERSION 3.6)
project(NumDB)

set(CMAKE_CXX_STANDARD 14)

include_directories(../include)
include_directories(../3rdparty/function_traits/include)
include_directories(../3rdparty/murmurhash2functor/include)

add_library(numdb numdb/fair_lru.cpp)
target_link_libraries(numdb murmurhash2)


