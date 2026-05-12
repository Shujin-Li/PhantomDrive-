cmake_minimum_required(VERSION 3.19)
project(untitled LANGUAGES CXX)

# 1. 在这里添加 Charts 模块
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Charts)

qt_standard_project_setup()

#添加源文件（确保文件名和路径正确）
qt_add_executable(untitled
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    DrivingReportWidget.h
    DrivingReportWidget.cpp
    learninghud.h
    learninghud.cpp
)

# 2. 在这里链接 Qt::Charts 库
target_link_libraries(untitled
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Charts
)

install(TARGETS untitled
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET untitled
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)