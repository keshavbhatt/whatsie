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

set_target_properties(whatsie PROPERTIES
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
