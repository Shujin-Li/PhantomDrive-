cmake_minimum_required(VERSION 3.20)

project(PhantomDrive VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/assets")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL Network Charts Xml Multimedia)
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
    include/PhantomDrive/gamemode/CustomTrackMode.h
    include/PhantomDrive/gamemode/VehicleSensor.h
    include/PhantomDrive/gamemode/CollisionDetector.h
    include/PhantomDrive/gamemode/DrivingDataStorage.h
    include/PhantomDrive/gamemode/DrivingDataCollector.h
    include/PhantomDrive/gamemode/Powerup.h
    include/PhantomDrive/gamemode/Powerups.h
    include/PhantomDrive/gamemode/PowerupManager.h
    include/PhantomDrive/gamemode/PowerupBox.h
    include/PhantomDrive/gamemode/PowerupWorldRuntime.h
    include/PhantomDrive/gamemode/AIOpponent.h
    include/PhantomDrive/gamemode/AIOpponentManager.h
    include/PhantomDrive/gamemode/SimpleAIOpponent.h
    include/PhantomDrive/gamemode/TrafficObject.h
    include/PhantomDrive/gamemode/TrafficLightObject.h
    include/PhantomDrive/gamemode/SpeedLimitSignObject.h
    include/PhantomDrive/gamemode/PedestrianCrossingObject.h
    include/PhantomDrive/gamemode/TrafficObjectManager.h
    include/PhantomDrive/scoring/ScoreReport.h
    include/PhantomDrive/scoring/ViolationConfig.h
    include/PhantomDrive/scoring/DrivingScoreCalculator.h
    include/PhantomDrive/scoring/TrafficRuleEnforcer.h
    include/PhantomDrive/scoring/AIAPIClient.h
    include/PhantomDrive/scoring/ScoreManager.h
    include/PhantomDrive/track/TrackTile.h
    include/PhantomDrive/track/Checkpoint.h
    include/PhantomDrive/track/TrackData.h
    include/PhantomDrive/track/BuiltInTrackFactory.h
    include/PhantomDrive/track/TrackIO.h
    include/PhantomDrive/track/TrackDatabase.h
    include/PhantomDrive/track/TrackManager.h
    include/PhantomDrive/track/TrackValidator.h
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
    include/PhantomDrive/physics/mcworldrenderer.hh
    include/PhantomDrive/physics/mcparticle.hh
    include/PhantomDrive/physics/mcshapeview.hh
    # MiniCore Asset 模块
    include/PhantomDrive/physics/Asset/mcassetmanager.hh
    include/PhantomDrive/physics/Asset/mcmeshconfigloader.hh
    include/PhantomDrive/physics/Asset/mcmeshloader.hh
    include/PhantomDrive/physics/Asset/mcmeshmanager.hh
    include/PhantomDrive/physics/Asset/mcmeshmetadata.hh
    include/PhantomDrive/physics/Asset/mcmeshobjectdata.hh
    include/PhantomDrive/physics/Asset/mcsurfaceconfigloader.hh
    include/PhantomDrive/physics/Asset/mcsurfacemanager.hh
    include/PhantomDrive/physics/Asset/mcsurfacemetadata.hh
    include/PhantomDrive/physics/Asset/mcsurfaceobjectdata.hh
    # MiniCore Text 模块
    include/PhantomDrive/physics/Text/mctexturefont.hh
    include/PhantomDrive/physics/Text/mctexturefontconfigloader.hh
    include/PhantomDrive/physics/Text/mctexturefontdata.hh
    include/PhantomDrive/physics/Text/mctexturefontmanager.hh
    include/PhantomDrive/physics/Text/mctextureglyph.hh
    include/PhantomDrive/physics/Text/mctexturetext.hh
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
    include/PhantomDrive/UI/learninghud.h
    include/PhantomDrive/UI/DrivingReportWidget.h
    include/PhantomDrive/UI/ABDrivingReportWidget.h
    include/PhantomDrive/UI/GameViewWidget.h
    include/PhantomDrive/UI/GameRenderState.h
    include/PhantomDrive/UI/TestVisualizationWindow.h
    include/PhantomDrive/UI/mainwindow.h
    include/PhantomDrive/UI/DrivingHistoryChartWidget.h
    include/PhantomDrive/UI/ThemeManager.h
    include/PhantomDrive/UI/SoundEffects.h
    include/PhantomDrive/UI/InteractiveFeedback.h
    include/PhantomDrive/UI/SoundManager.h
    include/PhantomDrive/UI/SoundGenerator.h
    include/PhantomDrive/UI/ArcadeHUD.h
    include/PhantomDrive/UI/CustomTrackEditorWidget.h
    include/PhantomDrive/core/GameEngine.h
    include/PhantomDrive/core/saveloadmanager.h
    include/PhantomDrive/core/VehiclePhysics.h
)

set(SOURCE_FILES
    src/gamemode/GameMode.cpp
    src/gamemode/ModeTransition.cpp
    src/gamemode/GameModeManager.cpp
    src/gamemode/ArcadeMode.cpp
    src/gamemode/LearningMode.cpp
    src/gamemode/CustomTrackMode.cpp
    src/gamemode/VehicleSensor.cpp
    src/gamemode/CollisionDetector.cpp
    src/gamemode/DrivingDataStorage.cpp
    src/gamemode/DrivingDataCollector.cpp
    src/gamemode/Powerup.cpp
    src/gamemode/Powerups.cpp
    src/gamemode/PowerupManager.cpp
    src/gamemode/PowerupBox.cpp
    src/gamemode/PowerupWorldRuntime.cpp
    src/gamemode/AIOpponent.cpp
    src/gamemode/AIOpponentManager.cpp
    src/gamemode/SimpleAIOpponent.cpp
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
    src/track/BuiltInTrackFactory.cpp
    src/track/TrackIO.cpp
    src/track/TrackDatabase.cpp
    src/track/TrackManager.cpp
    src/track/TrackValidator.cpp
    src/physics/MCPhysicsWorld.cpp
    src/physics/glew.c
    # src/physics/mcobjectfactory.cc
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
    # src/physics/mcobjectfactory.cc
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
    src/physics/mcworldrenderer.cc
    src/physics/mcparticle.cc
    src/physics/mcsurface.cc
    src/physics/mcsurfaceview.cc
    src/physics/mcsurfaceobjectrenderer.cc
    src/physics/mcsurfaceobjectrendererlegacy.cc
    src/physics/mcsurfaceparticle.cc
    src/physics/mcsurfaceparticlerenderer.cc
    src/physics/mcsurfaceparticlerendererlegacy.cc
    src/physics/mcshapeview.cc
    src/physics/mcmesh.cc
    src/physics/mcmeshview.cc
    src/physics/mcobjectrendererbase.cc
    src/physics/mcparticlerendererbase.cc
    src/physics/mcrenderlayer.cc
    src/physics/mccamera.cc
    src/physics/mcglambientlight.cc
    src/physics/mcgldiffuselight.cc
    src/physics/mcglmaterial.cc
    src/physics/mcglobjectbase.cc
    src/physics/mcglscene.cc
    src/physics/mcglshaderprogram.cc
    src/physics/mcassetmanager.cc
    src/physics/mcmeshconfigloader.cc
    src/physics/mcmeshloader.cc
    src/physics/mcmeshmanager.cc
    src/physics/mcmeshobjectdata.cc
    src/physics/mcsurfaceobjectdata.cc
    src/physics/mcsurfaceconfigloader.cc
    src/physics/mcsurfacemanager.cc
    src/physics/mctexturefont.cc
    src/physics/mctexturefontconfigloader.cc
    src/physics/mctexturefontdata.cc
    src/physics/mctexturefontmanager.cc
    src/physics/mctextureglyph.cc
    src/physics/mctexturetext.cc
    src/UI/mainwindow.cpp
    src/UI/learninghud.cpp
    src/UI/DrivingReportWidget.cpp
    src/UI/ABDrivingReportWidget.cpp
    src/UI/GameViewWidget.cpp
    src/UI/TestVisualizationWindow.cpp
    src/UI/DrivingHistoryChartWidget.cpp
    src/UI/InteractiveFeedback.cpp
    src/UI/SoundManager.cpp
    src/UI/SoundGenerator.cpp
    src/UI/ArcadeHUD.cpp
    src/UI/CustomTrackEditorWidget.cpp
    src/core/GameEngine.cpp
    src/core/saveloadmanager.cpp
    src/core/VehiclePhysics.cpp
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
    include/PhantomDrive/UI/mainwindow.h
    assets/mainwindow.ui
)

target_include_directories(PhantomDrive PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive
    # 子目录 include 路径
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive/gamemode
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive/track
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive/scoring
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive/UI
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive/core
    # MiniCore 物理引擎 include 路径（照抄 DustRacing2D 的配置）
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive/physics
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive/physics/Asset
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive/physics/Text
)

target_link_libraries(PhantomDrive PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::Network
    Qt6::Charts
    Qt6::Xml
    Qt6::Multimedia
    ${OPENGL_gl_LIBRARY}
)

set_target_properties(PhantomDrive PROPERTIES
    OUTPUT_NAME "PhantomDrive"
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    DEFINE_SYMBOL PHANTOMDRIVE_EXPORTS
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_compile_definitions(PhantomDrive PRIVATE
    PHANTOMDRIVE_EXPORTS
    GLEW_STATIC
    GLEW_NO_GLU
    __MC_NO_GLEW__
    __MC_QOPENGLFUNCTIONS__
    NOMINMAX
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

add_executable(PhantomDriveApp
    src/main.cpp
)

target_include_directories(PhantomDriveApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(PhantomDriveApp PRIVATE
    PhantomDrive
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Charts
)

set_target_properties(PhantomDriveApp PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    OUTPUT_NAME "PhantomDriveApp"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

install(TARGETS PhantomDriveApp
    RUNTIME DESTINATION bin
)

add_executable(test_main_window
    tests/test_main_window.cpp
)

target_include_directories(test_main_window PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(test_main_window PRIVATE
    PhantomDrive
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Charts
)

set_target_properties(test_main_window PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    OUTPUT_NAME "test_main_window"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_executable(test_visualization
    tests/test_visualization_simple.cpp
)

target_include_directories(test_visualization PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive/UI
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive/track
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(test_visualization PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

set_target_properties(test_visualization PROPERTIES
    AUTOMOC ON
    OUTPUT_NAME "test_visualization"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_executable(test_keyboard_input
    tests/test_keyboard_input.cpp
)

target_include_directories(test_keyboard_input PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive/UI
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhantomDrive/core
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(test_keyboard_input PRIVATE
    PhantomDrive
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

set_target_properties(test_keyboard_input PROPERTIES
    AUTOMOC ON
    OUTPUT_NAME "test_keyboard_input"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


