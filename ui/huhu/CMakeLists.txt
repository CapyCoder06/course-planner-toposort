# ui/huhu/CMakeLists.txt

set(UI_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

if (TARGET Qt6::Widgets)
    find_package(Qt6 COMPONENTS Widgets REQUIRED)
    qt_add_executable(course_planner_ui ${UI_SOURCES})
    target_link_libraries(course_planner_ui PRIVATE Qt6::Widgets course_core)
else()
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    add_executable(course_planner_ui ${UI_SOURCES})
    target_link_libraries(course_planner_ui PRIVATE Qt5::Widgets course_core)
endif()
