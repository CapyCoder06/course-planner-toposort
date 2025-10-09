cmake_minimum_required(VERSION 3.20)
project(course_planner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Qt6 trước, Qt5 fallback
find_package(Qt6 6.2 COMPONENTS Widgets QUIET)
if (Qt6_FOUND)
  set(QT_PACKAGE Qt6)
else()
  find_package(Qt5 5.12 COMPONENTS Widgets REQUIRED)
  set(QT_PACKAGE Qt5)
endif()

add_subdirectory(src)
add_subdirectory(ui/huhu)

# ---- OPTIONAL CLI (Wt) ----
option(BUILD_CLI "Build the Wt CLI target" OFF)  # ⬅⬅ mặc định OFF
if (BUILD_CLI)
  # Nếu thực sự muốn build CLI thì mới cần Wt
  find_package(Wt CONFIG QUIET)
  if (NOT Wt_FOUND)
    message(FATAL_ERROR "BUILD_CLI=ON nhưng không tìm thấy Wt. Cài Wt hoặc tắt BUILD_CLI.")
  endif()
  add_executable(course_cli src/cli/main.cpp)
  target_link_libraries(course_cli PRIVATE course_core Wt::Wt Wt::WtHttp)
endif()
