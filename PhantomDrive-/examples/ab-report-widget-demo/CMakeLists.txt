cmake_minimum_required(VERSION 3.20)

project(ABDrivingReportWidgetDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Charts)

add_executable(ab_report_widget_demo
    main.cpp
    ../../src/UI/ABDrivingReportWidget.cpp
    ../../include/PhantomDrive/UI/ABDrivingReportWidget.h
)

target_include_directories(ab_report_widget_demo PRIVATE
    ../../include
    ../../include/PhantomDrive
)

target_link_libraries(ab_report_widget_demo PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Charts
)
