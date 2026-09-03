# core: pure logic. Links Qt6::Core (+ Qt6::Gui for value types only).
# MUST NOT link Widgets, WebEngine, Network UI, or anything in ui/web/platform.

qt_add_library(whatsie_core STATIC
    app_lock.cpp
    app_lock.h
    chromium_flags.cpp
    chromium_flags.h
    graphics_fallback.cpp
    graphics_fallback.h
    gpu_storm_detector.cpp
    gpu_storm_detector.h
    spellcheck.cpp
    spellcheck.h
    connection_watchdog_policy.cpp
    connection_watchdog_policy.h
    downloads/download_model.cpp
    downloads/download_model.h
    downloads/file_naming.cpp
    downloads/file_naming.h
    log_sink.cpp
    log_sink.h
    logging.cpp
    logging.h
    navigation_policy.cpp
    navigation_policy.h
    notifications/dnd_controller.cpp
    notifications/dnd_controller.h
    notifications/identicon.cpp
    notifications/identicon.h
    notifications/notification.h
    notifications/notification_service.cpp
    notifications/notification_service.h
    render_crash_policy.cpp
    render_crash_policy.h
    settings/settings.cpp
    settings/settings.h
    settings/settings_keys.h
    storage_policy.cpp
    storage_policy.h
    theme/theme_service.cpp
    theme/theme_service.h
    unread_badge.cpp
    unread_badge.h
    zoom_policy.cpp
    zoom_policy.h
)
whatsie_set_warnings(whatsie_core)

target_include_directories(whatsie_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(whatsie_core
    PUBLIC
        Qt6::Core
        Qt6::Gui
)
