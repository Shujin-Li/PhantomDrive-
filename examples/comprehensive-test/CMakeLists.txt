cmake_minimum_required(VERSION 3.20)
project(ComprehensiveTest VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui)

# 查找父项目的源文件
set(PHANTOMDRIVE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

# 只包含需要的源文件
set(SOURCE_FILES
    ${PHANTOMDRIVE_DIR}/src/gamemode/VehicleSensor.cpp
    ${PHANTOMDRIVE_DIR}/src/gamemode/CollisionDetector.cpp
)

# 创建可执行文件
add_executable(comprehensive_test
    main.cpp
    ${SOURCE_FILES}
)

target_include_directories(comprehensive_test PRIVATE
    ${PHANTOMDRIVE_DIR}/include
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode
)

target_link_libraries(comprehensive_test PRIVATE
    Qt6::Core
    Qt6::Gui
)
