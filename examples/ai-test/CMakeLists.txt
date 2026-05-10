cmake_minimum_required(VERSION 3.16)

project(ai-example)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_executable(ai-example
    main.cpp
    ../../src/ai/AIOpponent.cpp
    ../../src/ai/AIOpponentFSM.cpp
)

target_include_directories(ai-example PRIVATE
    ../../include
)

target_link_libraries(ai-example PRIVATE
    Qt6::Core
)