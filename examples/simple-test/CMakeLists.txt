cmake_minimum_required(VERSION 3.20)
project(SimpleTest VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui)

set(PHANTOMDRIVE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

add_executable(simple_test
    main.cpp
)

target_include_directories(simple_test PRIVATE
    ${PHANTOMDRIVE_DIR}/include
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode
)

target_link_libraries(simple_test PRIVATE
    Qt6::Core
    Qt6::Gui
)
