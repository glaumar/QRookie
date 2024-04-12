cmake_minimum_required(VERSION 3.21.1)

option(LINK_INSIGHT "Link Qt Insight Tracker library" ON)
option(BUILD_QDS_COMPONENTS "Build design studio components" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(QRookie
    VERSION 0.2.1
    DESCRIPTION "Download and install Quest games from ROOKIE Public Mirror (like VRP Rookie Sideloader but for linux)"
    HOMEPAGE_URL https://github.com/glaumar/QRookie
    LANGUAGES CXX)

set(APPLICATION_NAME "QRookie")
set(APPLICATION_ID "io.github.glaumar.QRookie")
set(DESKTOP_FILE_NAME ${APPLICATION_ID}.desktop)
set(APPLICATION_ICON_BASE_NAME ${APPLICATION_ID})
set(METAINFO_FILE_NAME ${APPLICATION_ID}.metainfo.xml)
set(APP_BINARY_NAME "qrookie")

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Network Xml)
find_package(QCoro6 REQUIRED COMPONENTS Core Qml Network)
find_package(ECM 6.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEClangFormat)
include(KDEGitCommitHooks)

if (Qt6_VERSION VERSION_GREATER_EQUAL 6.3)
    qt_standard_project_setup()
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
configure_file(src/qrookie.h.in qrookie.h)

qt_add_executable(${APP_BINARY_NAME} src/main.cpp
                             src/vrp_public.cpp src/vrp_public.h
                             src/vrp_torrent.cpp src/vrp_torrent.h
                             src/vrp_manager.cpp src/vrp_manager.h
                             src/device_manager.cpp src/device_manager.h
                             src/http_downloader.cpp src/http_downloader.h
                             src/game_info_model.cpp src/game_info_model.h 
                             src/game_info.h)

qt_add_resources(${APP_BINARY_NAME} "configuration"
    PREFIX "/"
    FILES
        qtquickcontrols2.conf
)

target_link_libraries(${APP_BINARY_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Network
    Qt6::Xml
    QCoro6::Core
    QCoro6::Qml
    QCoro6::Network
)

include(${CMAKE_CURRENT_SOURCE_DIR}/qmlmodules)

install(TARGETS ${APP_BINARY_NAME} DESTINATION bin)
install(FILES ${DESKTOP_FILE_NAME} DESTINATION share/applications)
install(FILES icons/16x16/${APPLICATION_ID}.png DESTINATION share/icons/hicolor/16x16/apps)
install(FILES icons/32x32/${APPLICATION_ID}.png DESTINATION share/icons/hicolor/32x32/apps)
install(FILES icons/48x48/${APPLICATION_ID}.png DESTINATION share/icons/hicolor/48x48/apps)
install(FILES icons/64x64/${APPLICATION_ID}.png DESTINATION share/icons/hicolor/64x64/apps)
install(FILES icons/128x128/${APPLICATION_ID}.png DESTINATION share/icons/hicolor/128x128/apps)
install(FILES icons/256x256/${APPLICATION_ID}.png DESTINATION share/icons/hicolor/256x256/apps)
install(FILES icons/scalable/${APPLICATION_ID}.svg DESTINATION share/icons/hicolor/scalable/apps)
install(FILES ${METAINFO_FILE_NAME} DESTINATION share/metainfo)

# Add clang-format target
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.hpp *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)