# © 2019 Joseph Cameron - All Rights Reserved

cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/jfc-cmake/jfclib.cmake")

#option(JFC_BUILD_DEMO "Build the demo" ON)
#option(JFC_BUILD_DOCS "Build documentation" ON)
#option(JFC_BUILD_TESTS "Build unit tests" ON)

add_subdirectory(thirdparty)

jfc_project(executable #library
    NAME "gdkaudio"
    VERSION 0.0
    DESCRIPTION "game development kit graphics lib"
    C++_STANDARD 17
    C_STANDARD 99
    #TYPE STATIC

    PUBLIC_INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/

    PRIVATE_INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include
        ${CMAKE_CURRENT_SOURCE_DIR}/implementation/openal/include
        ${openal-soft_INCLUDE_DIR}

    LIBRARIES
        ${openal-soft_LIBRARIES}

    DEPENDENCIES
        "OpenAL"

    SOURCE_LIST
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/context.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/stb_vorbis.c
        
        ${CMAKE_CURRENT_SOURCE_DIR}/implementation/openal/src/openal_context.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/implementation/openal/src/openal_emitter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/implementation/openal/src/openal_simple_sound.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/implementation/openal/src/openal_stream.cpp
)

#[[if (JFC_BUILD_DEMO)
    add_subdirectory(demo)
endif()

if (JFC_BUILD_TESTS)
    add_subdirectory(test)
endif()

if (JFC_BUILD_DOCS)
    add_subdirectory(docs)
endif()]]

