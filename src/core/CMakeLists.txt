# core: pure logic. Links Qt6::Core (+ Qt6::Gui for value types only).
# MUST NOT link Widgets, WebEngine, Network UI, or anything in ui/web/platform.

qt_add_library(whatsie_core STATIC
    log_sink.cpp
    log_sink.h
    logging.cpp
    logging.h
    navigation_policy.cpp
    navigation_policy.h
    render_crash_policy.cpp
    render_crash_policy.h
    settings/settings.cpp
    settings/settings.h
    settings/settings_keys.h
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
