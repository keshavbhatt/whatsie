# core: pure logic. Links Qt6::Core (+ Qt6::Gui for value types only).
# MUST NOT link Widgets, WebEngine, Network UI, or anything in ui/web/platform.

qt_add_library(whatsie_core STATIC
    logging.cpp
    logging.h
    settings/settings.cpp
    settings/settings.h
    settings/settings_keys.h
)
whatsie_set_warnings(whatsie_core)

target_include_directories(whatsie_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(whatsie_core
    PUBLIC
        Qt6::Core
        Qt6::Gui
)
