cmake_minimum_required(VERSION 3.24)
project(whatsie LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set version
set(PROJECT_VERSION 5.0.0)
set(PROJECT_DESCRIPTION "Feature rich WhatsApp web client based on Qt WebEngine")

# Get Git information
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")

# Set Qt version - Require Qt 6.10 minimum
set(QT_VERSION_MAJOR 6)
set(QT_VERSION_MINOR 10)

# Set CMake policies
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required packages
find_package(Qt${QT_VERSION_MAJOR} ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR} REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    WebEngineWidgets
    Positioning
    Network
)

# Compiler flags
add_compile_options(
    -fstack-protector-strong
    -fstack-clash-protection
    -D_FORTIFY_SOURCE=3
    -D_GLIBCXX_ASSERTIONS
)

# Architecture-specific flags
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    add_compile_options(-mbranch-protection=standard)
endif()

# Release and Debug specific flags
add_compile_options($<$<CONFIG:Release>:-O2>)
add_compile_options($<$<CONFIG:Release>:-DQT_NO_DEBUG_OUTPUT>)
add_compile_options($<$<CONFIG:Debug>:-O1>)

# Installation prefix
if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX /usr)
endif()

set(BINDIR ${CMAKE_INSTALL_PREFIX}/bin)
set(DATADIR ${CMAKE_INSTALL_PREFIX}/share)

# Main application source files
set(SOURCES
    src/about.cpp
    src/automatictheme.cpp
    src/common.cpp
    src/dictionaries.cpp
    src/downloadmanagerwidget.cpp
    src/downloadwidget.cpp
    src/lock.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow_lock.cpp
    src/mainwindow_tray.cpp
    src/mainwindow_webengine.cpp
    src/permissiondialog.cpp
    src/rateapp.cpp
    src/settingswidget.cpp
    src/sunclock.cpp
    src/theme.cpp
    src/utils.cpp
    src/webenginepage.cpp
    src/webengineprofilemanager.cpp
    src/webview.cpp
    src/widgets/elidedlabel/elidedlabel.cpp
    src/widgets/scrolltext/scrolltext.cpp
    src/widgets/MoreApps/moreapps.cpp
)

set(HEADERS
    src/about.h
    src/autolockeventfilter.h
    src/automatictheme.h
    src/common.h
    src/def.h
    src/dictionaries.h
    src/downloadmanagerwidget.h
    src/downloadwidget.h
    src/lock.h
    src/mainwindow.h
    src/notificationpopup.h
    src/permissiondialog.h
    src/rateapp.h
    src/requestinterceptor.h
    src/settingsmanager.h
    src/settingswidget.h
    src/sunclock.hpp
    src/theme.h
    src/utils.h
    src/webenginepage.h
    src/webengineprofilemanager.h
    src/webview.h
    src/widgets/elidedlabel/elidedlabel.h
    src/widgets/scrolltext/scrolltext.h
    src/widgets/MoreApps/moreapps.h
)

set(UI_FILES
    src/about.ui
    src/automatictheme.ui
    src/certificateerrordialog.ui
    src/downloadmanagerwidget.ui
    src/downloadwidget.ui
    src/lock.ui
    src/passworddialog.ui
    src/permissiondialog.ui
    src/rateapp.ui
    src/settingswidget.ui
    src/widgets/MoreApps/moreapps.ui
)

set(RESOURCES
    src/icons.qrc
)

# SingleApplication library
set(SINGLEAPP_SOURCES
    src/singleapplication/singleapplication.cpp
    src/singleapplication/singleapplication_p.cpp
)

set(SINGLEAPP_HEADERS
    src/singleapplication/SingleApplication
    src/singleapplication/singleapplication.h
    src/singleapplication/singleapplication_p.h
)

# Create the main executable
add_executable(whatsie
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCES}
    ${SINGLEAPP_SOURCES}
    ${SINGLEAPP_HEADERS}
)

# Include directories
target_include_directories(whatsie PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/singleapplication
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/elidedlabel
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/scrolltext
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/MoreApps
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Link libraries
target_link_libraries(whatsie PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::WebEngineWidgets
    Qt${QT_VERSION_MAJOR}::Positioning
    Qt${QT_VERSION_MAJOR}::Network
    X11
)

# Define compiler definitions
target_compile_definitions(whatsie PRIVATE
    QAPPLICATION_CLASS=QApplication
    QT_DEPRECATED_WARNINGS
    GIT_HASH="${GIT_HASH}"
    GIT_BRANCH="${GIT_BRANCH}"
    BUILD_TIMESTAMP="${BUILD_TIMESTAMP}"
    VERSIONSTR="${PROJECT_VERSION}"
)

# Check for WebEngine spellchecker support
# Note: This check is simplified; you may need to enhance it based on your build environment
message(STATUS "Qt6 WebEngine spellchecker support: enabled (required)")

# Install the executable
install(TARGETS whatsie DESTINATION ${BINDIR})

# Install desktop file
install(FILES dist/linux/com.ktechpit.whatsie.desktop
    DESTINATION ${DATADIR}/applications
)

# Install appdata file
install(FILES dist/linux/com.ktechpit.whatsie.appdata.xml
    DESTINATION ${DATADIR}/metainfo
)

# Install license
install(FILES LICENSE
    DESTINATION ${DATADIR}/licenses/whatsie
)

# Install icons
install(FILES dist/linux/hicolor/16x16/apps/com.ktechpit.whatsie.png
    DESTINATION ${DATADIR}/icons/hicolor/16x16/apps
)
install(FILES dist/linux/hicolor/32x32/apps/com.ktechpit.whatsie.png
    DESTINATION ${DATADIR}/icons/hicolor/32x32/apps
)
install(FILES dist/linux/hicolor/64x64/apps/com.ktechpit.whatsie.png
    DESTINATION ${DATADIR}/icons/hicolor/64x64/apps
)
install(FILES dist/linux/hicolor/128x128/apps/com.ktechpit.whatsie.png
    DESTINATION ${DATADIR}/icons/hicolor/128x128/apps
)
install(FILES dist/linux/hicolor/256x256/apps/com.ktechpit.whatsie.png
    DESTINATION ${DATADIR}/icons/hicolor/256x256/apps
)
install(FILES dist/linux/hicolor/scalable/apps/com.ktechpit.whatsie.svg
    DESTINATION ${DATADIR}/icons/hicolor/scalable/apps
)
install(FILES dist/linux/hicolor/symbolic/apps/com.ktechpit.whatsie-symbolic.svg
    DESTINATION ${DATADIR}/icons/hicolor/symbolic/apps
)

# Dictionary handling
option(FLATPAK_BUILD "Build for Flatpak (skip dictionary installation)" OFF)

if(NOT FLATPAK_BUILD)
    # Find the qwebengine_convert_dict tool for Qt6
    if(QT_VERSION_MAJOR GREATER_EQUAL 6)
        find_program(QWEBENGINE_CONVERT_DICT
            NAMES qwebengine_convert_dict
            PATHS ${Qt6_DIR}/../../../libexec
        )
    endif()

    if(QWEBENGINE_CONVERT_DICT)
        message(STATUS "Found qwebengine_convert_dict: ${QWEBENGINE_CONVERT_DICT}")

        # Create output directory for compiled dictionaries
        set(DICTIONARIES_DIR ${CMAKE_CURRENT_BINARY_DIR}/qtwebengine_dictionaries)
        file(MAKE_DIRECTORY ${DICTIONARIES_DIR})

        # Get all dictionary files
        file(GLOB DICT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/dictionaries/*.dic")

        # Create custom target for dictionary compilation
        add_custom_target(build_dictionaries ALL)

        foreach(DICT_FILE ${DICT_FILES})
            get_filename_component(DICT_BASENAME ${DICT_FILE} NAME_WE)
            set(OUTPUT_FILE ${DICTIONARIES_DIR}/${DICT_BASENAME}.bdic)
            set(AFF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/dictionaries/${DICT_BASENAME}.aff)

            add_custom_command(
                OUTPUT ${OUTPUT_FILE}
                COMMAND ${QWEBENGINE_CONVERT_DICT} ${DICT_FILE} ${OUTPUT_FILE}
                DEPENDS ${DICT_FILE} ${AFF_FILE}
                COMMENT "Building dictionary: ${DICT_BASENAME}"
            )
            add_custom_target(dict_${DICT_BASENAME} ALL DEPENDS ${OUTPUT_FILE})
            add_dependencies(build_dictionaries dict_${DICT_BASENAME})
        endforeach()

        # Install compiled dictionaries
        install(DIRECTORY ${DICTIONARIES_DIR}/
            DESTINATION ${DATADIR}/org.keshavnrj.ubuntu/WhatSie
        )
    else()
        message(WARNING "qwebengine_convert_dict not found. Dictionary compilation will be skipped.")
        message(WARNING "You can still manually place compiled .bdic files in the runtime directory.")
    endif()
endif()

# Summary message
message(STATUS "")
message(STATUS "========== Whatsie Build Configuration ==========")
message(STATUS "Version:                  ${PROJECT_VERSION}")
message(STATUS "Install prefix:           ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Build type:               ${CMAKE_BUILD_TYPE}")
message(STATUS "Git Hash:                 ${GIT_HASH}")
message(STATUS "Git Branch:               ${GIT_BRANCH}")
message(STATUS "Build Timestamp:          ${BUILD_TIMESTAMP}")
message(STATUS "Flatpak Build:            ${FLATPAK_BUILD}")
message(STATUS "================================================")
message(STATUS "")

