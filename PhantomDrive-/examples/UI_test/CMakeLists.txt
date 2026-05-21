cmake_minimum_required(VERSION 3.19)
project(ui_test_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# 1. 添加 Qt 模块
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Charts Network)

# 2. 设置 PhantomDrive 路径
set(PHANTOMDRIVE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

# 3. 添加源文件（包含 PhantomDrive 源码）
set(SOURCE_FILES
    test_coach_report.cpp
    ${PHANTOMDRIVE_DIR}/src/scoring/ScoreReport.cpp
    ${PHANTOMDRIVE_DIR}/src/scoring/AIAPIClient.cpp
    ${PHANTOMDRIVE_DIR}/src/scoring/DrivingScoreCalculator.cpp
    ${PHANTOMDRIVE_DIR}/src/scoring/TrafficRuleEnforcer.cpp
    ${PHANTOMDRIVE_DIR}/src/scoring/ScoreManager.cpp
    ${PHANTOMDRIVE_DIR}/src/gamemode/TrafficObject.cpp
    ${PHANTOMDRIVE_DIR}/src/gamemode/TrafficLightObject.cpp
    ${PHANTOMDRIVE_DIR}/src/gamemode/SpeedLimitSignObject.cpp
    ${PHANTOMDRIVE_DIR}/src/gamemode/PedestrianCrossingObject.cpp
    ${PHANTOMDRIVE_DIR}/src/gamemode/TrafficObjectManager.cpp
)

# 4. 需要 MOC 的头文件
set(HEADERS_TO_MOC
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/scoring/AIAPIClient.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/scoring/ScoreManager.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode/TrafficObject.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode/TrafficLightObject.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode/SpeedLimitSignObject.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode/PedestrianCrossingObject.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode/TrafficObjectManager.h
)

# 5. 创建可执行文件
qt_add_executable(ui_test_example
    WIN32 MACOSX_BUNDLE
    ${SOURCE_FILES}
    ${HEADERS_TO_MOC}
)

# 6. 链接 Qt 库
target_link_libraries(ui_test_example
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Charts
        Qt6::Network
)

# 7. 包含路径
target_include_directories(ui_test_example PRIVATE
    ${PHANTOMDRIVE_DIR}/include
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/scoring
    ${PHANTOMDRIVE_DIR}/src
)

# 8. 定义导出符号
target_compile_definitions(ui_test_example PRIVATE
    PHANTOMDRIVE_STATIC_DEFINE
)

install(TARGETS ui_test_example
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
