cmake_minimum_required(VERSION 3.20)

project(PhantomDrive VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL Network)
find_package(OpenGL REQUIRED)

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
    include/PhantomDrive/gamemode/VehicleSensor.h
    include/PhantomDrive/gamemode/CollisionDetector.h
    include/PhantomDrive/gamemode/DrivingDataStorage.h
    include/PhantomDrive/gamemode/DrivingDataCollector.h
    include/PhantomDrive/gamemode/Powerup.h
    include/PhantomDrive/gamemode/Powerups.h
    include/PhantomDrive/gamemode/PowerupManager.h
    include/PhantomDrive/gamemode/PowerupBox.h
    include/PhantomDrive/gamemode/AIOpponent.h
    include/PhantomDrive/gamemode/AIOpponentManager.h
    include/PhantomDrive/gamemode/TrafficObject.h
    include/PhantomDrive/gamemode/TrafficLightObject.h
    include/PhantomDrive/gamemode/SpeedLimitSignObject.h
    include/PhantomDrive/gamemode/PedestrianCrossingObject.h
    include/PhantomDrive/gamemode/TrafficObjectManager.h
    include/PhantomDrive/scoring/ScoreReport.h
    include/PhantomDrive/scoring/DrivingScoreCalculator.h
    include/PhantomDrive/scoring/TrafficRuleEnforcer.h
    include/PhantomDrive/scoring/AIAPIClient.h
    include/PhantomDrive/scoring/ScoreManager.h
    include/PhantomDrive/track/TrackTile.h
    include/PhantomDrive/track/Checkpoint.h
    include/PhantomDrive/track/TrackData.h
    include/PhantomDrive/track/TrackIO.h
    include/PhantomDrive/track/TrackDatabase.h
    include/PhantomDrive/track/TrackManager.h
    include/PhantomDrive/physics/Physics.h
    include/PhantomDrive/physics/MCPhysicsWorld.h
    include/PhantomDrive/physics/mcbbox.hh
    include/PhantomDrive/physics/mcbbox3d.hh
    include/PhantomDrive/physics/mccast.hh
    include/PhantomDrive/physics/mccircleshape.hh
    include/PhantomDrive/physics/mccollisiondetector.hh
    include/PhantomDrive/physics/mccollisionevent.hh
    include/PhantomDrive/physics/mccontact.hh
    include/PhantomDrive/physics/mcdragforcegenerator.hh
    include/PhantomDrive/physics/mcedge.hh
    include/PhantomDrive/physics/mcevent.hh
    include/PhantomDrive/physics/mcforcegenerator.hh
    include/PhantomDrive/physics/mcforceregistry.hh
    include/PhantomDrive/physics/mcfrictiongenerator.hh
    include/PhantomDrive/physics/mcgravitygenerator.hh
    include/PhantomDrive/physics/mcimpulsegenerator.hh
    include/PhantomDrive/physics/mclogger.hh
    include/PhantomDrive/physics/mcmacros.hh
    include/PhantomDrive/physics/mcmathutil.hh
    include/PhantomDrive/physics/mcobbox.hh
    include/PhantomDrive/physics/mcobject.hh
    include/PhantomDrive/physics/mcobjectcomponent.hh
    include/PhantomDrive/physics/mcobjectdata.hh
    include/PhantomDrive/physics/mcobjectfactory.hh
    include/PhantomDrive/physics/mcobjectgrid.hh
    include/PhantomDrive/physics/mcoutofboundariesevent.hh
    include/PhantomDrive/physics/mcphysicscomponent.hh
    include/PhantomDrive/physics/mcrandom.hh
    include/PhantomDrive/physics/mcrectshape.hh
    include/PhantomDrive/physics/mcrecycler.hh
    include/PhantomDrive/physics/mcsegment.hh
    include/PhantomDrive/physics/mcseparationevent.hh
    include/PhantomDrive/physics/mcshape.hh
    include/PhantomDrive/physics/mcspringforcegenerator.hh
    include/PhantomDrive/physics/mcspringforcegenerator2dfast.hh
    include/PhantomDrive/physics/mctimerevent.hh
    include/PhantomDrive/physics/mctrigonom.hh
    include/PhantomDrive/physics/mctyperegistry.hh
    include/PhantomDrive/physics/mcvector2d.hh
    include/PhantomDrive/physics/mcvector3d.hh
    include/PhantomDrive/physics/mcvectoranimation.hh
    include/PhantomDrive/physics/mcworld.hh
    include/PhantomDrive/physics/mcshapeview.hh
    # MiniCore Graphics 模块（暂时注释，需要 GLM 库）
    # include/PhantomDrive/physics/mcglambientlight.hh
    # include/PhantomDrive/physics/mcgldiffuselight.hh
    # include/PhantomDrive/physics/mcglmaterial.hh
    # include/PhantomDrive/physics/mcglobjectbase.hh
    # include/PhantomDrive/physics/mcglscene.hh
    # include/PhantomDrive/physics/mcglshaderprogram.hh
    # include/PhantomDrive/physics/mcgltexcoord.hh
    # include/PhantomDrive/physics/mcglvertex.hh
    # include/PhantomDrive/physics/mcglcolor.hh
)

set(SOURCE_FILES
    src/gamemode/GameMode.cpp
    src/gamemode/ModeTransition.cpp
    src/gamemode/GameModeManager.cpp
    src/gamemode/ArcadeMode.cpp
    src/gamemode/LearningMode.cpp
    src/gamemode/VehicleSensor.cpp
    src/gamemode/CollisionDetector.cpp
    src/gamemode/DrivingDataStorage.cpp
    src/gamemode/DrivingDataCollector.cpp
    src/gamemode/Powerup.cpp
    src/gamemode/Powerups.cpp
    src/gamemode/PowerupManager.cpp
    src/gamemode/PowerupBox.cpp
    src/gamemode/AIOpponent.cpp
    src/gamemode/AIOpponentManager.cpp
    src/gamemode/TrafficObject.cpp
    src/gamemode/TrafficLightObject.cpp
    src/gamemode/SpeedLimitSignObject.cpp
    src/gamemode/PedestrianCrossingObject.cpp
    src/gamemode/TrafficObjectManager.cpp
    src/scoring/ScoreReport.cpp
    src/scoring/DrivingScoreCalculator.cpp
    src/scoring/TrafficRuleEnforcer.cpp
    src/scoring/AIAPIClient.cpp
    src/scoring/ScoreManager.cpp
    src/track/TrackTile.cpp
    src/track/Checkpoint.cpp
    src/track/TrackData.cpp
    src/track/TrackIO.cpp
    src/track/TrackDatabase.cpp
    src/track/TrackManager.cpp
    src/physics/MCPhysicsWorld.cpp
    src/physics/glew.c
    src/physics/mccircleshape.cc
    src/physics/mccollisiondetector.cc
    src/physics/mccollisionevent.cc
    src/physics/mccontact.cc
    src/physics/mcdragforcegenerator.cc
    src/physics/mcevent.cc
    src/physics/mcforcegenerator.cc
    src/physics/mcforceregistry.cc
    src/physics/mcfrictiongenerator.cc
    src/physics/mcgravitygenerator.cc
    src/physics/mcimpulsegenerator.cc
    src/physics/mclogger.cc
    src/physics/mcmathutil.cc
    src/physics/mcobject.cc
    src/physics/mcobjectcomponent.cc
    src/physics/mcobjectdata.cc
    src/physics/mcobjectfactory.cc
    src/physics/mcobjectgrid.cc
    src/physics/mcoutofboundariesevent.cc
    src/physics/mcphysicscomponent.cc
    src/physics/mcrandom.cc
    src/physics/mcrectshape.cc
    src/physics/mcseparationevent.cc
    src/physics/mcshape.cc
    src/physics/mcspringforcegenerator2dfast.cc
    src/physics/mcspringforcegenerator.cc
    src/physics/mctimerevent.cc
    src/physics/mctrigonom.cc
    src/physics/mctyperegistry.cc
    src/physics/mcvectoranimation.cc
    src/physics/mcworld.cc
    # MiniCore Graphics 模块（暂时注释，需要 GLM 库）
    # src/physics/mcglambientlight.cc
    # src/physics/mcgldiffuselight.cc
    # src/physics/mcglmaterial.cc
    # src/physics/mcglobjectbase.cc
    # src/physics/mcglscene.cc
    # src/physics/mcglshaderprogram.cc
)

add_library(PhantomDrive SHARED
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_include_directories(PhantomDrive PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive
    # 子目录 include 路径
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive/gamemode
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive/track
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive/scoring
    # MiniCore 物理引擎 include 路径（照抄 DustRacing2D 的配置）
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive/physics
)

target_link_libraries(PhantomDrive PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::Network
    ${OPENGL_gl_LIBRARY}
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
