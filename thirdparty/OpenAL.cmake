# © 2019 Joseph Cameron - All Rights Reserved

# builds the contained source code release of openal soft (software implementation of the OpenAL audio device API).

if (CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(LIBTYPE STATIC) # var used by OpenAL to determine static/dynamic
    
    find_package(Threads REQUIRED)
endif()

add_subdirectory(${PROJECT_NAME})

get_property(_output_name TARGET "${PROJECT_NAME}" PROPERTY OUTPUT_NAME)

if (CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set_target_properties("${PROJECT_NAME}" PROPERTIES PREFIX "lib")
    set(libdir "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/lib${_output_name}.a;${CMAKE_THREAD_LIBS_INIT};${CMAKE_DL_LIBS}")
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(_fileType "${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(libdir "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/Release/${_output_name}${_fileType}")
endif()

jfc_set_dependency_symbols(
    INCLUDE_PATHS
        ${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}/include

     LIBRARIES
        ${libdir}
)
