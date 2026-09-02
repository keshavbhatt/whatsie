# ui: QtWidgets layer. May use everything. Nothing depends on it except app/.
# No .ui files — widgets are built in code (see CODING_STANDARDS.md §5).

find_package(Qt6 6.11 REQUIRED COMPONENTS Concurrent)

qt_add_library(whatsie_ui STATIC
    about_dialog.cpp
    about_dialog.h
    actions.cpp
    actions.h
    diagnostics.cpp
    diagnostics.h
    downloads_delegate.cpp
    downloads_delegate.h
    downloads_dialog.cpp
    downloads_dialog.h
    downloads_hub.cpp
    downloads_hub.h
    logging.cpp
    logging.h
    main_window.cpp
    main_window.h
    notification_hub.cpp
    notification_hub.h
    permission_list.cpp
    permission_list.h
    lock_screen.cpp
    lock_screen.h
    passcode_dialog.cpp
    passcode_dialog.h
    permission_prompt.cpp
    permission_prompt.h
    proxy_auth_dialog.cpp
    proxy_auth_dialog.h
    screen_picker_dialog.cpp
    screen_picker_dialog.h
    settings_dialog.cpp
    settings_dialog.h
    whatsapp_style.cpp
    whatsapp_style.h
    shortcuts_dialog.cpp
    shortcuts_dialog.h
    theme_applier.cpp
    theme_applier.h
    tray_controller.cpp
    tray_controller.h
    tray_notifier.cpp
    tray_notifier.h
)
whatsie_set_warnings(whatsie_ui)

set(WHATSIE_ICON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../resources/icons")
qt_add_resources(whatsie_ui "icons"
    PREFIX "/icons"
    BASE "${WHATSIE_ICON_DIR}"
    FILES
        "${WHATSIE_ICON_DIR}/whatsie.svg"
        "${WHATSIE_ICON_DIR}/whatsie-symbolic.svg"
        "${WHATSIE_ICON_DIR}/wa_bg.png"
        "${WHATSIE_ICON_DIR}/check.svg"
        "${WHATSIE_ICON_DIR}/chevron-down.svg"
        "${WHATSIE_ICON_DIR}/chevron-up.svg"
        "${WHATSIE_ICON_DIR}/hicolor/64x64/apps/com.ktechpit.whatsie.png"
        "${WHATSIE_ICON_DIR}/hicolor/128x128/apps/com.ktechpit.whatsie.png"
        "${WHATSIE_ICON_DIR}/hicolor/256x256/apps/com.ktechpit.whatsie.png"
)

target_link_libraries(whatsie_ui
    PUBLIC
        whatsie_web
        whatsie_platform
        Qt6::Widgets
        Qt6::Svg
        Qt6::Concurrent
)
