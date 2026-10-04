# © Joseph Cameron - All Rights Reserved

option(GDK_AUDIO_VENDORED_OPENAL
    "Build OpenAL Soft from the vendored source and link it statically" OFF)

if (NOT GDK_AUDIO_VENDORED_OPENAL)
    find_package(OpenAL CONFIG QUIET)
endif()

if (TARGET OpenAL::OpenAL)
    message(STATUS "gdk-audio: OpenAL from the system (${OpenAL_DIR}); GDK_AUDIO_VENDORED_OPENAL builds the vendored one")

    add_library(OpenAL INTERFACE)

    target_link_libraries(OpenAL INTERFACE OpenAL::OpenAL)

    return()
endif()

message(STATUS "gdk-audio: OpenAL built from the vendored source, linked statically")

set(LIBTYPE STATIC)

if (CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "Darwin")
    find_package(Threads REQUIRED)
endif()

add_subdirectory(OpenAL)

if (CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set_target_properties(OpenAL PROPERTIES PREFIX "lib")
endif()

target_include_directories(OpenAL INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/OpenAL/include>")

if (CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set_property(TARGET OpenAL APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
endif()
