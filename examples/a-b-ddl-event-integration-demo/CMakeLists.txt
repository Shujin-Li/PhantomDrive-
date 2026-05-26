cmake_minimum_required(VERSION 3.20)
project(ABDDLEventIntegrationDemo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network)

set(PHANTOMDRIVE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

set(SOURCE_FILES
    main.cpp
    ${PHANTOMDRIVE_DIR}/src/scoring/ScoreReport.cpp
    ${PHANTOMDRIVE_DIR}/src/scoring/DrivingScoreCalculator.cpp
    ${PHANTOMDRIVE_DIR}/src/scoring/TrafficRuleEnforcer.cpp
    ${PHANTOMDRIVE_DIR}/src/scoring/AIAPIClient.cpp
    ${PHANTOMDRIVE_DIR}/src/scoring/ScoreManager.cpp
    ${PHANTOMDRIVE_DIR}/src/gamemode/TrafficObject.cpp
    ${PHANTOMDRIVE_DIR}/src/gamemode/TrafficLightObject.cpp
    ${PHANTOMDRIVE_DIR}/src/gamemode/SpeedLimitSignObject.cpp
    ${PHANTOMDRIVE_DIR}/src/gamemode/PedestrianCrossingObject.cpp
    ${PHANTOMDRIVE_DIR}/src/gamemode/TrafficObjectManager.cpp
)

set(HEADERS_TO_MOC
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/scoring/AIAPIClient.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/scoring/ScoreManager.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode/IDrivingDataCollector.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode/TrafficObject.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode/TrafficLightObject.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode/SpeedLimitSignObject.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode/PedestrianCrossingObject.h
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode/TrafficObjectManager.h
)

add_executable(a_b_ddl_event_integration_demo
    ${SOURCE_FILES}
    ${HEADERS_TO_MOC}
)

target_compile_definitions(a_b_ddl_event_integration_demo PRIVATE
    PHANTOMDRIVE_STATIC_DEFINE
)

target_include_directories(a_b_ddl_event_integration_demo PRIVATE
    ${PHANTOMDRIVE_DIR}/include
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/gamemode
    ${PHANTOMDRIVE_DIR}/include/PhantomDrive/scoring
)

target_link_libraries(a_b_ddl_event_integration_demo PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Network
)
