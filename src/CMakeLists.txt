# src/CMakeLists.txt

# Quét tất cả mã nguồn core (graph/io/model/planner + PlannerService), loại trừ cli và ui
file(GLOB_RECURSE CORE_SOURCES
     CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

# Loại bỏ file của CLI và UI khỏi danh sách
foreach(p IN LISTS CORE_SOURCES)
  if(p MATCHES "/cli/" OR p MATCHES "/ui/")
    list(REMOVE_ITEM CORE_SOURCES "${p}")
  endif()
endforeach()

# Tạo thư viện core
add_library(course_core STATIC ${CORE_SOURCES})

# Export include path để UI/CLI include được headers trong src/
target_include_directories(course_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# nlohmann/json single-header (nếu bạn để ở external/nlohmann/json.hpp)
# Chỉnh lại đường dẫn nếu bạn để chỗ khác.
target_include_directories(course_core PUBLIC ${CMAKE_SOURCE_DIR}/external)

target_compile_features(course_core PUBLIC cxx_std_17)
