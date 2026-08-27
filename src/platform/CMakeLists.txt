# platform: OS-specific backends behind interfaces declared in core/.
# Implementations are selected here per OS — never with #ifdef inside classes
# (the factory is the one sanctioned switch).

qt_add_library(whatsie_platform STATIC
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
        linux/dbus_image.cpp
        linux/dbus_image.h
        linux/freedesktop_notifier.cpp
        linux/freedesktop_notifier.h
        linux/portal_notifier.cpp
        linux/portal_notifier.h
    )
    target_link_libraries(whatsie_platform PUBLIC Qt6::DBus)
endif()
