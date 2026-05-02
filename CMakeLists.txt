cmake_minimum_required(VERSION 3.20)

project(PhantomDrive VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(HEADER_FILES
    include/PhantomDrive/PhantomDrive_global.h
    include/PhantomDrive/PhantomDrive.h
    include/PhantomDrive/gamemode/DrivingData.h
    include/PhantomDrive/gamemode/IDrivingDataCollector.h
    include/PhantomDrive/gamemode/GameMode.h
    include/PhantomDrive/gamemode/ModeTransition.h
    include/PhantomDrive/gamemode/GameModeManager.h
    include/PhantomDrive/gamemode/ArcadeMode.h
    include/PhantomDrive/gamemode/LearningMode.h
    include/PhantomDrive/track/TrackTile.h
    include/PhantomDrive/track/Checkpoint.h
    include/PhantomDrive/track/TrackData.h
    include/PhantomDrive/track/TrackIO.h
    include/PhantomDrive/track/TrackDatabase.h
    include/PhantomDrive/track/TrackManager.h
)

set(SOURCE_FILES
    src/gamemode/GameMode.cpp
    src/gamemode/ModeTransition.cpp
    src/gamemode/GameModeManager.cpp
    src/gamemode/ArcadeMode.cpp
    src/gamemode/LearningMode.cpp
    src/track/TrackTile.cpp
    src/track/Checkpoint.cpp
    src/track/TrackData.cpp
    src/track/TrackIO.cpp
    src/track/TrackDatabase.cpp
    src/track/TrackManager.cpp
)

add_library(PhantomDrive SHARED
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_include_directories(PhantomDrive PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PhantomDrive PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

set_target_properties(PhantomDrive PROPERTIES
    OUTPUT_NAME "PhantomDrive"
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    DEFINE_SYMBOL PHANTOMDRIVE_EXPORTS
)

target_compile_definitions(PhantomDrive PRIVATE
    PHANTOMDRIVE_EXPORTS
)

install(TARGETS PhantomDrive
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/PhantomDrive/
    DESTINATION include/PhantomDrive
    FILES_MATCHING PATTERN "*.h"
)
