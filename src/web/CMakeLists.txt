# web: everything that touches Qt WebEngine. Depends on core only.
# Injected JavaScript lives in scripts/ as resource files — never as C++ strings.

qt_add_library(whatsie_web STATIC
    logging.cpp
    logging.h
    notification_presenter.cpp
    notification_presenter.h
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

# Script bundle. Files are added here as features land (see DOCS/FEATURES.md).
# qt_add_resources(whatsie_web "scripts"
#     PREFIX "/scripts"
#     BASE   "scripts"
#     FILES  scripts/bridge.js
# )

target_link_libraries(whatsie_web
    PUBLIC
        whatsie_core
        Qt6::WebEngineWidgets
        Qt6::WebChannel
)
