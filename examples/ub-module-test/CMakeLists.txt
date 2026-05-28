cmake_minimum_required(VERSION 3.20)

project(UBModuleTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia Charts)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)

set(SOURCES
    main.cpp
    UBDemoWindow.cpp
    ${ROOT_DIR}src/UI/SoundGenerator.cpp
    ${ROOT_DIR}src/UI/SoundManager.cpp
    ${ROOT_DIR}src/UI/InteractiveFeedback.cpp
    ${ROOT_DIR}src/UI/ArcadeHUD.cpp
)

set(HEADERS
    UBDemoWindow.h
    ${ROOT_DIR}include/PhantomDrive/UI/SoundEffects.h
    ${ROOT_DIR}include/PhantomDrive/UI/SoundGenerator.h
    ${ROOT_DIR}include/PhantomDrive/UI/SoundManager.h
    ${ROOT_DIR}include/PhantomDrive/UI/InteractiveFeedback.h
    ${ROOT_DIR}include/PhantomDrive/UI/ArcadeHUD.h
)

add_executable(ub_module_test ${SOURCES} ${HEADERS})

target_include_directories(ub_module_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ROOT_DIR}include
    ${ROOT_DIR}include/PhantomDrive
    ${ROOT_DIR}include/PhantomDrive/UI
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(ub_module_test PRIVATE PHANTOMDRIVE_STATIC_DEFINE)

target_link_libraries(ub_module_test PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Charts
)

set_target_properties(ub_module_test PROPERTIES
    OUTPUT_NAME "ub_module_test"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
