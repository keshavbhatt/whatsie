# ui: QtWidgets layer. May use everything. Nothing depends on it except app/.
# No .ui files — widgets are built in code (see CODING_STANDARDS.md §5).

qt_add_library(whatsie_ui STATIC
    logging.cpp
    logging.h
    main_window.cpp
    main_window.h
)
whatsie_set_warnings(whatsie_ui)

target_link_libraries(whatsie_ui
    PUBLIC
        whatsie_web
        whatsie_platform
        Qt6::Widgets
)
