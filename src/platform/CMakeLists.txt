# platform: OS-specific backends behind interfaces declared in core/.
# Implementations are selected here per OS — never with #ifdef inside classes
# (the notifier factory is the one sanctioned switch).

qt_add_library(whatsie_platform STATIC
    autostart.h
    crash_handler.h
    file_manager.h
    gpu_stderr_watch.h
    logging.cpp
    logging.h
    notifier_factory.cpp
    notifier_factory.h
    platform_info.cpp
    platform_info.h
)
whatsie_set_warnings(whatsie_platform)

target_link_libraries(whatsie_platform
    PUBLIC
        whatsie_core
)

if(LINUX)
    find_package(Qt6 6.11 REQUIRED COMPONENTS DBus)
    target_sources(whatsie_platform PRIVATE
        linux/autostart_xdg.cpp
        linux/crash_handler_posix.cpp
        linux/dbus_image.cpp
        linux/dbus_image.h
        linux/file_manager_dbus.cpp
        linux/gpu_stderr_watch.cpp
        linux/freedesktop_notifier.cpp
        linux/freedesktop_notifier.h
        linux/portal_notifier.cpp
        linux/portal_notifier.h
    )
    target_link_libraries(whatsie_platform PUBLIC Qt6::DBus)
else()
    target_sources(whatsie_platform PRIVATE
        crash_handler_generic.cpp
        file_manager_generic.cpp
        gpu_stderr_watch_generic.cpp
    )
endif()
