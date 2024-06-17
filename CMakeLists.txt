cmake_minimum_required(VERSION 3.21.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(QRookie
    VERSION 0.2.4
    DESCRIPTION "Download and install Quest games from ROOKIE Public Mirror (like VRP Rookie Sideloader but for linux)"
    HOMEPAGE_URL https://github.com/glaumar/QRookie
    LANGUAGES CXX)

set(APPLICATION_NAME "QRookie")
set(APPLICATION_ID "io.github.glaumar.QRookie")
set(DESKTOP_FILE_NAME ${APPLICATION_ID}.desktop)
set(APPLICATION_ICON_BASE_NAME ${APPLICATION_ID})
set(METAINFO_FILE_NAME ${APPLICATION_ID}.metainfo.xml)
set(APP_BINARY_NAME "QRookie")

set(CMAKE_AUTOMOC ON)

find_package(ECM 6.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

if(APPLE)
    add_definitions(-DMACOS)

    if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm64" OR CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        set(MAC_ARCH "arm64")
        set(CMAKE_OSX_ARCHITECTURES "arm64")
    else()
        set(MAC_ARCH "i386")
        set(CMAKE_OSX_ARCHITECTURES "x86_64")
    endif()

    set(PROJECT_DIR ${CMAKE_SOURCE_DIR})
    set(BUILD_DIR "${CMAKE_BINARY_DIR}")
    set(INSTALL_DIR "${PROJECT_DIR}/macOs/dependencies/install_${MAC_ARCH}")
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${INSTALL_DIR}/qcoro")
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${INSTALL_DIR}/kirigami")

    message("INSTALL_DIR: ${INSTALL_DIR}")
    message("CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Xml DBus)
find_package(Qt6 REQUIRED COMPONENTS Qml QmlWorkerScript)
find_package(Qt6 REQUIRED COMPONENTS Quick QuickLayouts QuickControls2 QuickTemplates2 Widgets Svg)
find_package(QCoro6 REQUIRED COMPONENTS Core Qml Network)
find_package(KF6Kirigami)

include(KDEClangFormat)
include(KDEGitCommitHooks)

if (Qt6_VERSION VERSION_GREATER_EQUAL 6.3)
    qt_standard_project_setup()
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
configure_file(src/qrookie.h.in qrookie.h)

qt_add_executable(${APP_BINARY_NAME} 
    src/main.cpp
                             src/vrp_public.cpp src/vrp_public.h
                             src/vrp_torrent.cpp src/vrp_torrent.h
                             src/vrp_manager.cpp src/vrp_manager.h
                             src/device_manager.cpp src/device_manager.h
                             src/http_downloader.cpp src/http_downloader.h
                             src/game_info_model.cpp src/game_info_model.h
                             src/game_info.h
    src/app_settings.cpp src/app_settings.h
)

qt_add_resources(${APP_BINARY_NAME} "configuration"
    PREFIX "/"
    FILES
        qtquickcontrols2.conf
)

target_link_libraries(${APP_BINARY_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::QmlWorkerScript
    Qt6::Quick
    Qt6::QuickLayouts
    Qt6::QuickControls2
    Qt6::QuickTemplates2
    Qt6::Network
    Qt6::Xml
    Qt6::DBus
    Qt6::Widgets
    Qt6::Svg
    QCoro6::Network
    QCoro6::Core
    QCoro6::Coro
    QCoro6::Qml
    KF6Kirigami
    KF6::Kirigami
)

include(${CMAKE_CURRENT_SOURCE_DIR}/qmlmodules)

if(APPLE)
    set_target_properties(${APP_BINARY_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/macOs/Info.plist
    )
    add_custom_command(TARGET ${APP_BINARY_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${BUILD_DIR}/QRookie.app/Contents/Resources"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/icons/icon.icns" "${BUILD_DIR}/QRookie.app/Contents/Resources/icon.icns"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${INSTALL_DIR}/kirigami/lib/qml"
            "${BUILD_DIR}/QRookie.app/Contents/Resources/kirigami"
    )

    set_target_properties(${APP_BINARY_NAME} PROPERTIES
        INSTALL_RPATH "@loader_path/../Frameworks"
    )

else()
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
endif()

# Add clang-format target
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.hpp *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)