cmake_minimum_required(VERSION 3.1)
project(NumDbTests)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

include_directories(../include)
include_directories(../3rdparty/function_traits/include)
include_directories(../3rdparty/murmurhash2functor/include)

add_executable(unit_tests
        unit/fair_lru_test.cpp
        unit/fixed_hashtable_fair_lru_test.cpp
        unit/splay_tree_test.cpp)
target_link_libraries(unit_tests numdb ${GTEST_BOTH_LIBRARIES} pthread)

add_executable(bench benchmark/bench.cpp)
target_link_libraries(bench numdb benchmark pthread)

add_test(UnitTests unit_tests)
