cmake_minimum_required(VERSION 3.16)
project(course_planner CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -------- Core library --------
set(PLANNER_SOURCES
  src/io/Loader.cpp
  src/io/Writer.cpp
  src/graph/CourseGraph.cpp
  src/graph/TopoSort.cpp
  src/graph/CycleDiagnosis.cpp
  src/planner/LongestPathDag.cpp
  src/planner/TermAssigner.cpp
  src/planner/Hints.cpp # có thì giữ, không có có thể xóa dòng này
)

# Thêm PlanConstraints.cpp nếu tồn tại (tránh lỗi thiếu file)
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/model/PlanConstraints.cpp")
  list(APPEND PLANNER_SOURCES src/model/PlanConstraints.cpp)
endif()

add_library(planner ${PLANNER_SOURCES})

target_include_directories(planner PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/model
  ${CMAKE_CURRENT_SOURCE_DIR}/external
)

# -------- GoogleTest --------
include(FetchContent)
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(planner_tests
  tests/writer_explain_test.cpp
)
target_link_libraries(planner_tests PRIVATE planner GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(planner_tests)

# -------- CLI --------
add_executable(planner_cli src/cli/main.cpp)
target_link_libraries(planner_cli PRIVATE planner)
target_include_directories(planner_cli PRIVATE src)
