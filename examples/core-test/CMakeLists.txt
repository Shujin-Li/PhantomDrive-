# PhantomDrive Core Test CMakeLists.txt
# 测试 U-B 模块: AI教练、存档、图表、HUD

cmake_minimum_required(VERSION 3.20)

project(PhantomDriveCoreTest VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找 Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Charts Network)

# 路径设置
set(PHANTOM_DRIVE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(PHANTOM_DRIVE_INCLUDE ${PHANTOM_DRIVE_ROOT}/include)
set(PHANTOM_DRIVE_SRC ${PHANTOM_DRIVE_ROOT}/src)

# ==================== PhantomDriveCore 静态库 ====================
set(CORE_HEADERS
    # 全局
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/PhantomDrive_global.h

    # Core
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/core/saveloadmanager.h
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/core/datamodels.h

    # Scoring (评分模块)
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/scoring/ScoreReport.h
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/scoring/DrivingScoreCalculator.h
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/scoring/AIAPIClient.h
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/scoring/TrafficRuleEnforcer.h
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/scoring/ViolationConfig.h

    # GameMode (基础类型)
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/gamemode/DrivingData.h

    # UI
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/UI/ThemeManager.h
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/UI/DrivingReportWidget.h
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/UI/DrivingHistoryChartWidget.h
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/UI/learninghud.h
)

set(CORE_SOURCES
    # Core
    ${PHANTOM_DRIVE_SRC}/core/saveloadmanager.cpp

    # Scoring
    ${PHANTOM_DRIVE_SRC}/scoring/ScoreReport.cpp
    ${PHANTOM_DRIVE_SRC}/scoring/DrivingScoreCalculator.cpp
    ${PHANTOM_DRIVE_SRC}/scoring/AIAPIClient.cpp
    ${PHANTOM_DRIVE_SRC}/scoring/TrafficRuleEnforcer.cpp

    # UI
    ${PHANTOM_DRIVE_SRC}/UI/DrivingReportWidget.cpp
    ${PHANTOM_DRIVE_SRC}/UI/DrivingHistoryChartWidget.cpp
    ${PHANTOM_DRIVE_SRC}/UI/learninghud.cpp
)

add_library(PhantomDriveCore STATIC
    ${CORE_HEADERS}
    ${CORE_SOURCES}
)

target_include_directories(PhantomDriveCore PUBLIC
    ${PHANTOM_DRIVE_INCLUDE}
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/core
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/scoring
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/gamemode
    ${PHANTOM_DRIVE_INCLUDE}/PhantomDrive/UI
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(PhantomDriveCore PRIVATE
    PHANTOMDRIVE_STATIC_DEFINE
)

target_link_libraries(PhantomDriveCore PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
)

# ==================== 测试可执行文件 ====================
add_executable(core-test
    main.cpp
)

target_link_libraries(core-test PRIVATE
    PhantomDriveCore
    Qt6::Core
    Qt6::Gui
)

# ==================== U-B 模块测试 ====================
add_executable(test-ub-module
    test_ub_module.cpp
)

target_compile_definitions(test-ub-module PRIVATE
    PHANTOMDRIVE_STATIC_DEFINE
)

target_link_libraries(test-ub-module PRIVATE
    PhantomDriveCore
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
)

# ==================== 输出配置 ====================
set_target_properties(core-test test-ub-module PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ==================== Windows DLL 复制 ====================
if(WIN32)
    add_custom_command(TARGET test-ub-module POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt6::Core>
            $<TARGET_FILE_DIR:test-ub-module>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt6::Gui>
            $<TARGET_FILE_DIR:test-ub-module>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt6::Widgets>
            $<TARGET_FILE_DIR:test-ub-module>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt6::Charts>
            $<TARGET_FILE_DIR:test-ub-module>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt6::Network>
            $<TARGET_FILE_DIR:test-ub-module>
    )
endif()

message(STATUS "========================================")
message(STATUS "  PhantomDrive Core Test - U-B Module")
message(STATUS "========================================")
message(STATUS "  AI Coach: Enabled (Mock/API)")
message(STATUS "  Scoring: Full integration")
message(STATUS "========================================")
