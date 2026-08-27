# app: the QApplication subclass, single-instance guard, CLI parsing.
# Version metadata is generated from CMake, never hand-written.

configure_file(version.h.in "${CMAKE_CURRENT_BINARY_DIR}/generated/app/version.h" @ONLY)

qt_add_library(whatsie_app STATIC
    application.cpp
    application.h
)
whatsie_set_warnings(whatsie_app)

target_include_directories(whatsie_app PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(whatsie_app
    PUBLIC
        whatsie_ui
)
