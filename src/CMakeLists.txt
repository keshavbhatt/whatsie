# Layered libraries. Dependency direction is enforced by target_link_libraries:
#
#   ui ──► web ──► core
#   │        │
#   └──► platform ─► core
#
# Nothing links to `ui` except the executable (via `app`).

add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(web)
add_subdirectory(ui)
add_subdirectory(app)

qt_add_executable(whatsie
    main.cpp
)
whatsie_set_warnings(whatsie)
target_link_libraries(whatsie PRIVATE whatsie_app)

if(LINUX)
    # QtWebEngine drags in libproxy, whose duktape/snappy backends are not always
    # resolvable at link time in the KDE Qt6 content snaps — on arm64 the SDK
    # update left libduktape.so.207 off the linker search path, so libproxy's
    # `duk_error_raw` broke the final link. Those symbols come from shared
    # libraries and resolve at runtime, so allow undefined symbols originating in
    # shared libraries; undefined references from our own objects still error.
    target_link_options(whatsie PRIVATE -Wl,--allow-shlib-undefined)
endif()

if(WIN32)
    # Embed the application icon and version information in the executable.
    string(REPLACE "." "," WHATSIE_VERSION_RC "${PROJECT_VERSION}")
    set(WHATSIE_VERSION_RC "${WHATSIE_VERSION_RC},0")
    configure_file("${CMAKE_SOURCE_DIR}/dist/windows/whatsie.rc.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/whatsie.rc" @ONLY)
    target_sources(whatsie PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/whatsie.rc")
endif()

set_target_properties(whatsie PROPERTIES
    # Export symbols (-rdynamic) so the crash handler's backtrace shows function
    # names instead of bare addresses.
    ENABLE_EXPORTS ON
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    # Pure QtWidgets app: no QML anywhere, skip the qmlimportscanner finalizer
    # (it also fails to run from the snap SDK on non-Ubuntu hosts).
    QT_QML_MODULE_NO_IMPORT_SCAN TRUE
)

install(TARGETS whatsie
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Interface translations (issue #171). Compile src/i18n/<lang>.ts to .qm and
# embed them at :/i18n so main.cpp can load one for the chosen or system locale.
# Needs the Qt Linguist tools; where they are absent (some SDKs) the build
# simply ships an English-only interface.
find_package(Qt6 QUIET COMPONENTS LinguistTools)
if(Qt6LinguistTools_FOUND)
    qt_add_translations(whatsie
        SOURCE_TARGETS whatsie_core whatsie_platform whatsie_web whatsie_ui whatsie_app
        TS_FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/i18n/de.ts
            ${CMAKE_CURRENT_SOURCE_DIR}/i18n/es.ts
            ${CMAKE_CURRENT_SOURCE_DIR}/i18n/fr.ts
            ${CMAKE_CURRENT_SOURCE_DIR}/i18n/it.ts
            ${CMAKE_CURRENT_SOURCE_DIR}/i18n/pt_BR.ts
            ${CMAKE_CURRENT_SOURCE_DIR}/i18n/ru.ts
            ${CMAKE_CURRENT_SOURCE_DIR}/i18n/zh_CN.ts
        RESOURCE_PREFIX "/i18n"
    )
else()
    message(WARNING
        "whatsie: Qt Linguist tools not found; the interface will be English only. "
        "Install them (qt6-l10n-tools on Debian/Ubuntu) to build translations.")
endif()
