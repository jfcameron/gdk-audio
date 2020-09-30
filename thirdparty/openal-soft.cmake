# © 2019 Joseph Cameron - All Rights Reserved

# builds the contained source code release of openal soft (software implementation of the OpenAL audio device API).

#set(LIBTYPE STATIC) # var used by OpenAL to determine static/dynamic

add_subdirectory(${PROJECT_NAME})

set(PROJECT_NAME OpenAL)

#set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "lib")

get_property(_prefix TARGET ${PROJECT_NAME} PROPERTY PREFIX)
get_property(_suffix TARGET ${PROJECT_NAME} PROPERTY SUFFIX)
get_property(_output_name TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME)

set(libdir "${PROJECT_BINARY_DIR}/openal-soft/${_prefix}${_output_name}.a")

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(_fileType "${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(libdir "${PROJECT_BINARY_DIR}/openal-soft/Release/${_prefix}${_output_name}${_fileType}")
endif()

jfc_set_dependency_symbols(
    INCLUDE_PATHS
        "${CMAKE_CURRENT_LIST_DIR}/openal-soft/include"

    LIBRARIES
        "${libdir}"
)

