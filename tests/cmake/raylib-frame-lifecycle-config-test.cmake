if(NOT DEFINED PROJECT_BINARY_DIR OR PROJECT_BINARY_DIR STREQUAL "")
    message(FATAL_ERROR "PROJECT_BINARY_DIR is required")
endif()

set(cache_file "${PROJECT_BINARY_DIR}/CMakeCache.txt")
if(NOT EXISTS "${cache_file}")
    message(FATAL_ERROR "CMake cache was not found: ${cache_file}")
endif()

file(STRINGS "${cache_file}" frame_control_entries REGEX "^SUPPORT_CUSTOM_FRAME_CONTROL:BOOL=")
if(NOT frame_control_entries STREQUAL "SUPPORT_CUSTOM_FRAME_CONTROL:BOOL=OFF")
    message(
        FATAL_ERROR
        "SUPPORT_CUSTOM_FRAME_CONTROL must be OFF; found '${frame_control_entries}'"
    )
endif()
