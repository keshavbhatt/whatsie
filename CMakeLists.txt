cmake_minimum_required(VERSION 3.21)

project(whatsie
    VERSION 6.0.0
    DESCRIPTION "WhatsApp Web desktop client built with Qt WebEngine"
    HOMEPAGE_URL "https://github.com/keshavbhatt/whatsie"
    LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(WHATSIE_BUILD_TESTS "Build unit tests" ON)
option(WHATSIE_WERROR "Treat warnings as errors" OFF)
option(WHATSIE_ENABLE_CLANG_TIDY "Run clang-tidy during the build" OFF)

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Warnings)
include(Version)

# ---------------------------------------------------------------------------
# Qt 6.11 — the version shipped by the kde-qt6-core24-sdk snap and the
# Flathub KDE runtime, i.e. what every distributed build actually runs on.
# See scripts/dev-build.sh.
# ---------------------------------------------------------------------------
find_package(Qt6 6.11 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    WebEngineWidgets
    WebChannel
    Network
    Concurrent
    Svg
)
qt_standard_project_setup()
include(SnapSdkWorkaround)

# Qt hygiene: forces explicit string conversions and keyword-free signal syntax.
add_compile_definitions(
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_CAST_FROM_BYTEARRAY
    QT_NO_KEYWORDS
    QT_NO_FOREACH
    QT_DISABLE_DEPRECATED_UP_TO=0x061100
    QT_USE_QSTRINGBUILDER
)

if(WHATSIE_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy requested but not found")
    endif()
endif()

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------
add_subdirectory(src)

# ---------------------------------------------------------------------------
# Desktop integration (Linux): .desktop, AppStream metainfo, hicolor icons.
# ---------------------------------------------------------------------------
if(LINUX)
    include(GNUInstallDirs)
    install(FILES dist/linux/com.ktechpit.whatsie.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES dist/linux/com.ktechpit.whatsie.metainfo.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
    install(DIRECTORY src/resources/icons/hicolor
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)

    # FEATURES L1 / ADR-017: convert hunspell dictionaries to Chromium .bdic at
    # build time and bundle them under share/whatsie/qtwebengine_dictionaries.
    # Best-effort: if the tool or the dictionaries are absent (e.g. this dev
    # host), it silently produces nothing and spell check falls back to a no-op.
    # CI (kde-neon-6) has both, so the packaged app ships real dictionaries.
    # The tool ships in the Qt WebEngine libexec dir, which varies by distro and
    # (in the kde-neon-6 snap SDK) lives under the SDK mount. Search broadly and,
    # as a last resort, glob the SDK/Qt trees. All of this is logged so a failed
    # lookup is diagnosable from the CI output.
    file(GLOB _whatsie_convert_glob
        /snap/*/current/usr/lib/*/qt6/libexec/qwebengine_convert_dict
        /snap/*/current/usr/lib/qt6/libexec/qwebengine_convert_dict
        /usr/lib/*/qt6/libexec/qwebengine_convert_dict
        /usr/lib/qt6/libexec/qwebengine_convert_dict)
    find_program(WHATSIE_CONVERT_DICT
        NAMES qwebengine_convert_dict
        HINTS ${_whatsie_convert_glob}
        PATHS /usr/lib/qt6 /usr/lib/x86_64-linux-gnu/qt6 /usr/lib/aarch64-linux-gnu/qt6
              /usr/lib/qt6/libexec /usr/libexec /usr/libexec/qt6
        PATH_SUFFIXES libexec qt6/libexec)
    set(WHATSIE_HUNSPELL_DIR "/usr/share/hunspell" CACHE PATH "hunspell .aff/.dic directory")
    set(_whatsie_dicts_out "${CMAKE_BINARY_DIR}/qtwebengine_dictionaries")
    message(STATUS "whatsie: qwebengine_convert_dict = ${WHATSIE_CONVERT_DICT}")
    if(EXISTS "${WHATSIE_HUNSPELL_DIR}")
        file(GLOB _whatsie_hunspell_list "${WHATSIE_HUNSPELL_DIR}/*.dic")
        message(STATUS "whatsie: hunspell dir ${WHATSIE_HUNSPELL_DIR} has "
                       "${_whatsie_hunspell_list}")
    else()
        message(STATUS "whatsie: hunspell dir ${WHATSIE_HUNSPELL_DIR} does not exist")
    endif()
    if(WHATSIE_CONVERT_DICT AND EXISTS "${WHATSIE_HUNSPELL_DIR}")
        file(GLOB _whatsie_affs "${WHATSIE_HUNSPELL_DIR}/*.aff")
        file(MAKE_DIRECTORY "${_whatsie_dicts_out}")
        foreach(_aff ${_whatsie_affs})
            get_filename_component(_base "${_aff}" NAME_WE)
            if(EXISTS "${WHATSIE_HUNSPELL_DIR}/${_base}.dic")
                string(REPLACE "_" "-" _dictname "${_base}")
                execute_process(
                    COMMAND "${WHATSIE_CONVERT_DICT}"
                            "${WHATSIE_HUNSPELL_DIR}/${_base}.dic"
                            "${_whatsie_dicts_out}/${_dictname}.bdic"
                    RESULT_VARIABLE _rc OUTPUT_QUIET ERROR_QUIET)
                if(_rc EQUAL 0)
                    message(STATUS "whatsie: bundled spell-check dictionary ${_dictname}")
                endif()
            endif()
        endforeach()
    else()
        message(STATUS "whatsie: spell-check dictionaries not built "
                       "(need qwebengine_convert_dict and ${WHATSIE_HUNSPELL_DIR})")
    endif()
    if(EXISTS "${_whatsie_dicts_out}")
        install(DIRECTORY "${_whatsie_dicts_out}"
            DESTINATION ${CMAKE_INSTALL_DATADIR}/whatsie
            FILES_MATCHING PATTERN "*.bdic")
    endif()
endif()

if(WHATSIE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
