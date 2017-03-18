cmake_minimum_required(VERSION 3.1)
project(NumDbTests)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -g")
enable_testing()
find_package(GTest REQUIRED)

include_directories(../../include)
include_directories(../../3rdparty/include)
include_directories(../../3rdparty/murmurhash2functor/include)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(unit_tests fair_lru_test.cpp fixed_hashtable_fair_lru_test.cpp)
target_link_libraries(unit_tests murmurhash2 ${GTEST_BOTH_LIBRARIES} pthread)

add_test(UnitTests unit_tests)
