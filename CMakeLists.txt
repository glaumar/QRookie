cmake_minimum_required(VERSION 3.21.1)

option(LINK_INSIGHT "Link Qt Insight Tracker library" ON)
option(BUILD_QDS_COMPONENTS "Build design studio components" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(QRookie
    VERSION 0.1.0
    DESCRIPTION "Download and install Quest games on linux (like VRP Rookie Sideloader but for linux)"
    HOMEPAGE_URL https://github.com/glaumar/QRookie
    LANGUAGES CXX)

set(APPLICATION_NAME "QRookie")
set(APPLICATION_ID "io.github.glaumar.QRookie")
set(DESKTOP_FILE_NAME ${APPLICATION_ID}.desktop)
set(APPLICATION_ICON_BASE_NAME ${APPLICATION_ID})
set(METAINFO_FILE_NAME ${APPLICATION_ID}.metainfo.xml)
set(APP_BINARY_NAME "qrookie")

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Network)
find_package(QCoro6 REQUIRED COMPONENTS Core)


if (Qt6_VERSION VERSION_GREATER_EQUAL 6.3)
    qt_standard_project_setup()
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
configure_file(src/qrookie.h.in qrookie.h)

qt_add_executable(${APP_BINARY_NAME} src/main.cpp
                             src/vrp_public.cpp src/vrp_public.h
                             src/vrp_downloader.cpp src/vrp_downloader.h
                             src/device_manager.cpp src/device_manager.h
                             src/game_info.h
                             src/librclone_wrapper.h
                             src/rclone_result.h)

qt_add_resources(${APP_BINARY_NAME} "configuration"
    PREFIX "/"
    FILES
        qtquickcontrols2.conf
)

# librclone
target_include_directories(${APP_BINARY_NAME} PRIVATE "third/rclone/")
target_link_directories(${APP_BINARY_NAME} PRIVATE "third/rclone/")
target_link_libraries(${APP_BINARY_NAME} PRIVATE librclone.a)

target_link_libraries(${APP_BINARY_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Network
    QCoro6::Core
    librclone.a
)

include(${CMAKE_CURRENT_SOURCE_DIR}/qmlmodules)

include(GNUInstallDirs)
install(TARGETS ${APP_BINARY_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# make IDEs aware of the QML import path
# set(QML_IMPORT_PATH ${PROJECT_BINARY_DIR}/qml CACHE PATH
#     "Path to the custom QML components defined by the project")
