# SPDX-FileCopyrightText: 2026 Keshav Bhatt (Ktechpit)
# SPDX-License-Identifier: MIT
#
# KtechpitApps: "More apps from Ktechpit" cards for the About dialog, the What's new sheet
# and a full list. Add it to an app with add_subdirectory() and link ktechpit_apps.
cmake_minimum_required(VERSION 3.21)
project(ktechpit_apps VERSION 1.1.0 LANGUAGES CXX)

if(NOT Qt6_FOUND OR NOT TARGET Qt6::Widgets OR NOT TARGET Qt6::Network)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Network)
endif()

option(KTECHPIT_APPS_BUILD_TESTS "Build the KtechpitApps tests" OFF)
option(KTECHPIT_APPS_BUILD_DEMO "Build the KtechpitApps demo window" OFF)

add_library(ktechpit_apps STATIC
    AppCatalog.h AppCatalog.cpp
    AppLinks.h AppLinks.cpp
    AppCard.h AppCard.cpp
    AppStrip.h AppStrip.cpp
    MoreAppsView.h MoreAppsView.cpp
    MoreAppsDialog.h MoreAppsDialog.cpp
    ktechpit_apps.qrc
)
set_target_properties(ktechpit_apps PROPERTIES
    AUTOMOC ON
    AUTORCC ON
)
target_compile_features(ktechpit_apps PUBLIC cxx_std_17)
target_include_directories(ktechpit_apps PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ktechpit_apps PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)

if(KTECHPIT_APPS_BUILD_TESTS)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    enable_testing()
    add_executable(tst_ktechpit_apps tests/tst_ktechpit_apps.cpp)
    set_target_properties(tst_ktechpit_apps PROPERTIES AUTOMOC ON)
    target_link_libraries(tst_ktechpit_apps PRIVATE ktechpit_apps Qt6::Test)
    add_test(NAME tst_ktechpit_apps COMMAND tst_ktechpit_apps)
    set_tests_properties(tst_ktechpit_apps PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()

if(KTECHPIT_APPS_BUILD_DEMO)
    add_executable(ktechpit_apps_demo demo/main.cpp)
    target_link_libraries(ktechpit_apps_demo PRIVATE ktechpit_apps)
endif()
