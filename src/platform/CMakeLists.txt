# platform: OS-specific backends behind interfaces declared in core/.
# Implementations are selected here per OS — never with #ifdef inside classes.

qt_add_library(whatsie_platform STATIC
    platform_info.cpp
    platform_info.h
)
whatsie_set_warnings(whatsie_platform)

target_link_libraries(whatsie_platform
    PUBLIC
        whatsie_core
)

# Per-OS sources are appended here as backends land, e.g.:
# if(LINUX)
#     target_sources(whatsie_platform PRIVATE linux/autostart_xdg.cpp linux/autostart_xdg.h)
# elseif(WIN32)
#     target_sources(whatsie_platform PRIVATE windows/autostart_registry.cpp ...)
# endif()
