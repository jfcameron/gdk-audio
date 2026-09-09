# © Joseph Cameron - All Rights Reserved

cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/jfc-cmake/jfclib.cmake")

project(gdkaudio
    VERSION 0.0
    DESCRIPTION "game development kit graphics lib")

option(JFC_BUILD_DEMO "Build the demo" ON)
option(JFC_BUILD_DOCS "Build documentation" ON)
option(JFC_BUILD_TESTS "Build unit tests" ON)

set(_jfc_outer_build_tests "${JFC_BUILD_TESTS}")
set(_jfc_outer_build_docs "${JFC_BUILD_DOCS}")
set(_jfc_outer_build_demo "${JFC_BUILD_DEMO}")

set(JFC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(JFC_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(JFC_BUILD_DEMO OFF CACHE BOOL "" FORCE)

add_subdirectory(extern)

set(JFC_BUILD_TESTS "${_jfc_outer_build_tests}" CACHE BOOL "" FORCE)
set(JFC_BUILD_DOCS "${_jfc_outer_build_docs}" CACHE BOOL "" FORCE)
set(JFC_BUILD_DEMO "${_jfc_outer_build_demo}" CACHE BOOL "" FORCE)

file(GLOB_RECURSE _gdk_audio_own_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/impl/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/impl/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/gdk/audio/*.h")

foreach(_source ${_gdk_audio_own_sources})
    file(READ "${_source}" _contents)

    if (_contents MATCHES "#include[ \t]*<gdk/audio\\.h>")
        message(FATAL_ERROR
            "${_source} includes the umbrella <gdk/audio.h>. It is for consumers; include the "
            "individual headers this file actually needs.")
    endif()
endforeach()

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/permissive->)
else()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-pedantic>)
    endif()

    include(CheckCXXCompilerFlag)

    foreach (_strict_flag "-Werror=c++20-extensions" "-Werror=c++23-extensions")
        string(MAKE_C_IDENTIFIER "HAS${_strict_flag}" _strict_flag_symbol)

        check_cxx_compiler_flag("${_strict_flag}" ${_strict_flag_symbol})

        if (${_strict_flag_symbol})
            add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${_strict_flag}>)
        endif()
    endforeach()
endif()

add_library(gdkaudio STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/openal/src/openal_context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/openal/src/openal_emitter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/openal/src/openal_listener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/openal/src/openal_scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/openal/src/openal_simple_emitter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/openal/src/openal_stream_emitter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/exception.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/insert_effects.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pcm_sound.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vorbis_sound.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stb_vorbis.c
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/openal/src/openal_microphone.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/null/src/null_context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/null/src/null_emitter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/null/src/null_listener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/null/src/null_microphone.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/null/src/null_scene.cpp)

add_library(gdk::audio ALIAS gdkaudio)

set_target_properties(gdkaudio PROPERTIES EXPORT_NAME audio)

set_target_properties(gdkaudio PROPERTIES PREFIX "lib")

jfc_generate_cmake_header()

target_include_directories(gdkaudio
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/impl/openal/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/impl/null/include>"
        "$<INSTALL_INTERFACE:include>"
    PRIVATE
        "${PROJECT_BINARY_DIR}/generated_include/"
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    )

target_compile_features(gdkaudio PUBLIC cxx_std_20)

set_property(TARGET gdkaudio PROPERTY C_STANDARD 99)

target_link_libraries(gdkaudio PUBLIC
    gdkmath
    jfc-smart_handles
    OpenAL)

if (JFC_BUILD_DEMO)
    add_subdirectory(demo)
endif()

if (JFC_BUILD_TESTS)
    add_subdirectory(test)
endif()

if (JFC_BUILD_DOCS)
    #add_subdirectory(docs)
endif()

