cmake_minimum_required(VERSION 3.1)
project(NumDbTests)

set(CMAKE_CXX_STANDARD 14)

enable_testing()
find_package(GTest REQUIRED)

include_directories(../../include)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(fair_lru_test fair_lru_test.cpp)
target_link_libraries(fair_lru_test ${GTEST_BOTH_LIBRARIES} pthread)

add_test(FairLRUTests fair_lru_test)
