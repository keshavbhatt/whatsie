# Derives version metadata for the build.
#
# WHATSIE_VERSION        -> "5.0.0" (from project(VERSION))
# WHATSIE_GIT_REVISION   -> short hash or "unknown"
#
# These are handed to the code via a generated header (see src/app/CMakeLists.txt),
# never via string literals in sources.

set(WHATSIE_VERSION "${PROJECT_VERSION}")

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE WHATSIE_GIT_REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()
if(NOT WHATSIE_GIT_REVISION)
    set(WHATSIE_GIT_REVISION "unknown")
endif()

message(STATUS "whatsie ${WHATSIE_VERSION} (${WHATSIE_GIT_REVISION})")
