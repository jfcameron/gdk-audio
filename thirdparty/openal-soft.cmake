# © 2019 Joseph Cameron - All Rights Reserved

# builds the contained source code release of openal soft (software implementation of the OpenAL audio device API).

add_subdirectory(${PROJECT_NAME})

jfc_set_dependency_symbols(
    INCLUDE_PATHS
        "${CMAKE_CURRENT_LIST_DIR}/openal-soft/include"

    LIBRARIES
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/libopenal.1.19.1.dylib"
)

