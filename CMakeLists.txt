cmake_minimum_required(VERSION 3.21.1)

option(LINK_INSIGHT "Link Qt Insight Tracker library" ON)
option(BUILD_QDS_COMPONENTS "Build design studio components" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(qrookieApp
    VERSION 0.1.0
    DESCRIPTION "VRP Rookie Sideloader for linux (unofficial)"
    HOMEPAGE_URL https://github.com/glaumar/QRookie
    LANGUAGES CXX)

set(APPLICATION_NAME "QRookie")
set(APPLICATION_ID "io.github.glaumar.QRookie")
set(DESKTOP_FILE_NAME ${APPLICATION_ID}.desktop)
set(APPLICATION_ICON_BASE_NAME ${APPLICATION_ID})
set(METAINFO_FILE_NAME ${APPLICATION_ID}.metainfo.xml)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Qml Quick Network)

if (Qt6_VERSION VERSION_GREATER_EQUAL 6.3)
    qt_standard_project_setup()
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
configure_file(src/qrookie.h.in qrookie.h)

qt_add_executable(qrookieApp src/main.cpp
                             src/vrp_public.cpp src/vrp_public.h
                             src/vrp_downloader.cpp src/vrp_downloader.h
                             src/game_info.h
                             src/librclone_wrapper.h
                             src/rclone_result.h)

qt_add_resources(qrookieApp "configuration"
    PREFIX "/"
    FILES
        qtquickcontrols2.conf
)

# librclone
target_include_directories(qrookieApp PRIVATE "third/rclone/")
target_link_directories(qrookieApp PRIVATE "third/rclone/")

target_link_libraries(qrookieApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Network
    librclone.a
)

if (BUILD_QDS_COMPONENTS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/qmlcomponents)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/qmlmodules)

if (LINK_INSIGHT)
    include(${CMAKE_CURRENT_SOURCE_DIR}/insight)
endif ()

include(GNUInstallDirs)
install(TARGETS qrookieApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# make IDEs aware of the QML import path
set(QML_IMPORT_PATH ${PROJECT_BINARY_DIR}/qml CACHE PATH
    "Path to the custom QML components defined by the project")
