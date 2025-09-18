cmake_minimum_required(VERSION 3.16)
project(course_planner CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(planner
  src/graph/CourseGraph.cpp
  src/io/Loader.cpp
)
target_include_directories(planner PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# GoogleTest
include(FetchContent)
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(planner_tests
  tests/smoke_test.cpp
  tests/loader_test.cpp
)
target_link_libraries(planner_tests PRIVATE planner GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(planner_tests)

# CLI
add_executable(planner_cli src/cli/main.cpp)
target_link_libraries(planner_cli PRIVATE planner)
