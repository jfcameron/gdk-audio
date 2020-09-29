# © 2019 Joseph Cameron - All Rights Reserved

# builds the contained source code release of openal soft (software implementation of the OpenAL audio device API).

add_subdirectory(${PROJECT_NAME})

set(PROJECT_NAME OpenAL)

get_property(_prefix TARGET ${PROJECT_NAME} PROPERTY PREFIX)
get_property(_suffix TARGET ${PROJECT_NAME} PROPERTY SUFFIX)
get_property(_output_name TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME)
set(_fileType "${CMAKE_STATIC_LIBRARY_SUFFIX}")


#TODO: if file is not in root then if file is in debug move it to root if not then move from release to root.

if (NOT EXISTS "${PROJECT_BINARY_DIR}/openal-soft/${_prefix}${_output_name}${_fileType}")
#    if (EXISTS "${PROJECT_BINARY_DIR}/openal-soft/Debug/${_prefix}${_output_name}${_fileType}")
 #       file (COPY "${PROJECT_BINARY_DIR}/openal-soft/Debug/${_prefix}${_output_name}${_fileType}" "${PROJECT_BINARY_DIR}/openal-soft/${_prefix}${_output_name}${_fileType}")
  #  endif()
    if (EXISTS "${PROJECT_BINARY_DIR}/openal-soft/Release/${_prefix}${_output_name}${_fileType}")
        file (COPY "${PROJECT_BINARY_DIR}/openal-soft/Release/${_prefix}${_output_name}${_fileType}" "${PROJECT_BINARY_DIR}/openal-soft/${_prefix}${_output_name}${_fileType}")
    endif()
endif()


jfc_set_dependency_symbols(
    INCLUDE_PATHS
        "${CMAKE_CURRENT_LIST_DIR}/openal-soft/include"

    LIBRARIES
        "${PROJECT_BINARY_DIR}/openal-soft/Release/${_prefix}${_output_name}${_fileType}"
)

