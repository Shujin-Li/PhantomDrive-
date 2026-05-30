cmake_minimum_required(VERSION 3.20)
project(LearningModeTest VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui)

# 查找父项目的源文件
set(PHANTOMDRIVE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

# 源文件
set(SOURCE_FILES
    main.cpp

    ${PHANTOMDRIVE_DIR}/src/gamemode/TrafficObject.cpp
    ${PHANTOMDRIVE_DIR}/src/gamemode/TrafficLightObject.cpp
    ${PHANTOMDRIVE_DIR}/src/gamemode/SpeedLimitSignObject.cpp
    ${PHANTOMDRIVE_DIR}/src/gamemode/PedestrianCrossingObject.cpp
    ${PHANTOMDRIVE_DIR}/src/gamemode/TrafficObjectManager.cpp
)

# 显式包含需要 MOC 处理的头文件
set(HEADERS_TO_MOC
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode/TrafficObject.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode/TrafficLightObject.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode/SpeedLimitSignObject.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode/PedestrianCrossingObject.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode/TrafficObjectManager.h
)

# 创建可执行文件
add_executable(learning_mode_test
    ${SOURCE_FILES}
    ${HEADERS_TO_MOC}
)

target_include_directories(learning_mode_test PRIVATE
    ${PHANTOMDRIVE_DIR}/include
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode
)

target_compile_definitions(learning_mode_test PRIVATE PHANTOMDRIVE_STATIC_DEFINE)

target_link_libraries(learning_mode_test PRIVATE
    Qt6::Core
    Qt6::Gui
)
