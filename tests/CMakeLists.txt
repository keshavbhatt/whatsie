find_package(Qt6 6.11 REQUIRED COMPONENTS Test Qml)

# Environment every test runs with. Offscreen so the suite works headless and
# in CI; GPU off because WebEngine has nothing to draw to.
set(WHATSIE_TEST_ENV
    "QT_QPA_PLATFORM=offscreen"
    "QT_LOGGING_RULES=whatsie.*.debug=false"
    "QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu"
    "QT_FORCE_STDERR_LOGGING=1"
)

# Dev builds against the snap SDK run against the kf6-core24 content snap
# (set by scripts/dev-build.sh). Bake that runtime into the test environment
# so a plain `ctest` works from the host shell.
set(WHATSIE_SNAP_RUNTIME "" CACHE PATH "kf6-core24 runtime snap root for running tests (dev builds)")
set(WHATSIE_SNAP_RUNTIME_LIB_PATH "" CACHE STRING "LD_LIBRARY_PATH for the snap runtime (dev builds)")
if(WHATSIE_SNAP_RUNTIME)
    list(APPEND WHATSIE_TEST_ENV
        "LD_LIBRARY_PATH=${WHATSIE_SNAP_RUNTIME_LIB_PATH}"
        "QT_PLUGIN_PATH=${WHATSIE_SNAP_RUNTIME}/usr/lib/x86_64-linux-gnu/qt6/plugins"
        "QTWEBENGINEPROCESS_PATH=${WHATSIE_SNAP_RUNTIME}/usr/lib/qt6/libexec/QtWebEngineProcess"
        "QTWEBENGINE_RESOURCES_PATH=${WHATSIE_SNAP_RUNTIME}/usr/share/qt6/resources"
    )
endif()

# whatsie_add_test(<name> <source> <link-target> [GUI])
#   GUI tests need a QApplication; they are labelled "gui" and get a longer timeout.
function(whatsie_add_test name source link_target)
    qt_add_executable(${name} ${source})
    set_target_properties(${name} PROPERTIES QT_QML_MODULE_NO_IMPORT_SCAN TRUE)
    whatsie_set_warnings(${name})
    target_link_libraries(${name} PRIVATE ${link_target} Qt6::Test)
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES ENVIRONMENT "${WHATSIE_TEST_ENV}")
    if(ARGC GREATER 3 AND ARGV3 STREQUAL "GUI")
        set_tests_properties(${name} PROPERTIES LABELS "gui" TIMEOUT 120)
    else()
        set_tests_properties(${name} PROPERTIES LABELS "unit" TIMEOUT 30)
    endif()
endfunction()

# core
whatsie_add_test(tst_settings            tst_settings.cpp            whatsie_core)
whatsie_add_test(tst_zoom_policy         tst_zoom_policy.cpp         whatsie_core)
whatsie_add_test(tst_unread_badge        tst_unread_badge.cpp        whatsie_core GUI)
whatsie_add_test(tst_navigation_policy   tst_navigation_policy.cpp   whatsie_core)
whatsie_add_test(tst_render_crash_policy tst_render_crash_policy.cpp whatsie_core)
whatsie_add_test(tst_connection_watchdog tst_connection_watchdog.cpp whatsie_core)
whatsie_add_test(tst_graphics_fallback   tst_graphics_fallback.cpp   whatsie_core)
whatsie_add_test(tst_spellcheck          tst_spellcheck.cpp          whatsie_core GUI)
whatsie_add_test(tst_log_sink            tst_log_sink.cpp            whatsie_core)
whatsie_add_test(tst_theme_service       tst_theme_service.cpp       whatsie_core GUI)
whatsie_add_test(tst_download_model      tst_download_model.cpp      whatsie_core)
whatsie_add_test(tst_storage_and_flags   tst_storage_and_flags.cpp   whatsie_core)
whatsie_add_test(tst_identicon           tst_identicon.cpp           whatsie_core GUI)
whatsie_add_test(tst_notification_service tst_notification_service.cpp whatsie_core GUI)
# platform
if(LINUX)
    whatsie_add_test(tst_freedesktop_notifier tst_freedesktop_notifier.cpp whatsie_platform GUI)
endif()
# web
whatsie_add_test(tst_user_agent          tst_user_agent.cpp          whatsie_web)
whatsie_add_test(tst_web_scripts         tst_web_scripts.cpp         whatsie_web  GUI)
whatsie_add_test(tst_proxy               tst_proxy.cpp               whatsie_web GUI)
whatsie_add_test(tst_permissions         tst_permissions.cpp         whatsie_web  GUI)
target_link_libraries(tst_web_scripts PRIVATE Qt6::Qml)
# app / ui
whatsie_add_test(tst_cli_options         tst_cli_options.cpp         whatsie_app)
whatsie_add_test(tst_single_instance     tst_single_instance.cpp     whatsie_app)
whatsie_add_test(tst_smoke               tst_smoke.cpp               whatsie_ui  GUI)
