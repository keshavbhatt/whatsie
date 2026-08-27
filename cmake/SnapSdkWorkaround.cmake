# Dev builds against the kde-qt6-core24-sdk snap on a non-Ubuntu host: some
# imported Qt targets export the SDK's generic /usr/include (Ubuntu glibc
# headers), which shadow the host toolchain's libc headers and break
# libstdc++. Strip that one directory from every imported target; the Qt
# headers proper live under .../include/<arch>/qt6 and are unaffected.
# Harmless in snapcraft builds (the dir then equals the real sysroot's).
#
# Must be included AFTER find_package(Qt6 ...).

if(CMAKE_PREFIX_PATH MATCHES "/snap/kde-qt6-core24-sdk/")
    get_property(_whatsie_imported DIRECTORY PROPERTY IMPORTED_TARGETS)
    foreach(_tgt IN LISTS _whatsie_imported)
        get_target_property(_incs ${_tgt} INTERFACE_INCLUDE_DIRECTORIES)
        if(_incs)
            set(_filtered "")
            foreach(_dir IN LISTS _incs)
                if(NOT _dir MATCHES "/snap/kde-qt6-core24-sdk/[^/]+/usr/include$")
                    list(APPEND _filtered "${_dir}")
                endif()
            endforeach()
            set_property(TARGET ${_tgt} PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${_filtered}")
        endif()
    endforeach()
    unset(_whatsie_imported)
    unset(_filtered)
    unset(_incs)
endif()
