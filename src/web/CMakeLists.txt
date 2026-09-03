# web: everything that touches Qt WebEngine. Depends on core only.
# Injected JavaScript lives in scripts/ as resource files — never as C++ strings.

qt_add_library(whatsie_web STATIC
    bridge.cpp
    bridge.h
    clipboard_fix.cpp
    clipboard_fix.h
    error_page.cpp
    error_page.h
    file_drop.cpp
    file_drop.h
    full_screen_hint.cpp
    full_screen_hint.h
    download_controller.cpp
    download_controller.h
    logging.cpp
    logging.h
    notification_presenter.cpp
    notification_presenter.h
    permission_controller.cpp
    permission_controller.h
    popup_window.cpp
    popup_window.h
    proxy.cpp
    proxy.h
    script_bundle.cpp
    script_bundle.h
    user_agent.cpp
    user_agent.h
    web_page.cpp
    web_page.h
    web_profile.cpp
    web_profile.h
    web_view.cpp
    web_view.h
)
whatsie_set_warnings(whatsie_web)

# Script bundle (see scripts/README.md). Added as features are approved.
qt_add_resources(whatsie_web "scripts"
    PREFIX "/scripts"
    BASE "scripts"
    FILES
        scripts/bootstrap.js
        scripts/theme-control.js
        scripts/nav-settings.js
        scripts/storage-persist.js
        scripts/connection-watchdog.js
        scripts/sw-recovery.js
        scripts/privacy-blur.js
        scripts/linked-device-name.js
        scripts/file-drop.js
)

target_link_libraries(whatsie_web
    PUBLIC
        whatsie_core
        whatsie_platform
        Qt6::WebEngineWidgets
        Qt6::WebChannel
        Qt6::Network
        Qt6::Concurrent
)
