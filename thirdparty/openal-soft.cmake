# © 2019 Joseph Cameron - All Rights Reserved

# builds the contained source code release of openal soft (software implementation of the OpenAL audio device API).

add_subdirectory(${PROJECT_NAME})

set(PROJECT_NAME OpenAL)

#[[add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${PROJECT_BINARY_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>")]]


get_property(_prefix TARGET ${PROJECT_NAME} PROPERTY PREFIX)
get_property(_suffix TARGET ${PROJECT_NAME} PROPERTY SUFFIX)
get_property(_output_name TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME)
set(_fileType "${CMAKE_STATIC_LIBRARY_SUFFIX}")

jfc_set_dependency_symbols(
    INCLUDE_PATHS
        "${CMAKE_CURRENT_LIST_DIR}/openal-soft/include"

    LIBRARIES
        "${PROJECT_BINARY_DIR}/openal-soft/${_prefix}${_output_name}${_fileType}"
)

