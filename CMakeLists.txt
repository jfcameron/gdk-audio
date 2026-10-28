# © Joseph Cameron - MIT License

cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

if (TARGET gdkaudio)
    return()
endif()

project(gdk-audio
    VERSION 1.0
    DESCRIPTION "the gdk audio interface: scenes of emitters and a listener, sounds, insert effects and microphones, and a null implementation"
    LANGUAGES C CXX)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/jfc-cmake/jfclib.cmake")

option(GDKAUDIO_BUILD_TESTS "Build unit tests" ${PROJECT_IS_TOP_LEVEL})
option(GDKAUDIO_BUILD_DOCS "Build documentation" OFF)

if (GDKAUDIO_BUILD_DOCS AND NOT GDKAUDIO_BUILD_TESTS)
    add_subdirectory(docs)

    return()
endif()

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

add_subdirectory(extern)

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

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(gdkaudio STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/exception.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/insert_effects.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pcm_sound.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/impl/null/src/null_context.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/impl/null/src/null_emitter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/impl/null/src/null_listener.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/impl/null/src/null_microphone.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/impl/null/src/null_scene.cpp")

add_library(gdk::audio ALIAS gdkaudio)

set_target_properties(gdkaudio PROPERTIES EXPORT_NAME audio PREFIX "lib")

target_include_directories(gdkaudio
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/impl/null/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/include")

target_compile_features(gdkaudio PUBLIC cxx_std_20)

target_link_libraries(gdkaudio PUBLIC gdkmath)

add_library(gdkaudio_vorbis STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ext/vorbis_sound.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ext/stb_vorbis.c")

add_library(gdk::audio_vorbis ALIAS gdkaudio_vorbis)

set_target_properties(gdkaudio_vorbis PROPERTIES EXPORT_NAME audio_vorbis PREFIX "lib" C_STANDARD 99)

target_compile_features(gdkaudio_vorbis PUBLIC cxx_std_20)

target_link_libraries(gdkaudio_vorbis PUBLIC gdkaudio)

target_compile_definitions(gdkaudio_vorbis PRIVATE STB_VORBIS_NO_STDIO=1)

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/ext/stb_vorbis.c" PROPERTIES
    COMPILE_OPTIONS "$<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-tautological-compare>;$<$<C_COMPILER_ID:MSVC>:/WX->")

add_library(gdkaudio_implementation INTERFACE)

target_include_directories(gdkaudio_implementation INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src/include")

target_link_libraries(gdkaudio_implementation INTERFACE gdkaudio)

add_library(gdkaudio_conformance INTERFACE)

target_sources(gdkaudio_conformance INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/test/conformance/context_conformance.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/test/conformance/emitter_conformance.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/test/conformance/scene_conformance.cpp")

target_include_directories(gdkaudio_conformance INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/test/conformance")

target_link_libraries(gdkaudio_conformance INTERFACE gdkaudio)

install(TARGETS gdkaudio gdkaudio_vorbis
    EXPORT gdk-audio-targets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    "${CMAKE_CURRENT_SOURCE_DIR}/impl/null/include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(EXPORT gdk-audio-targets
    FILE gdk-audio-targets.cmake
    NAMESPACE gdk::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gdk-audio")

write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/gdk-audio-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

file(WRITE "${PROJECT_BINARY_DIR}/gdk-audio-config.cmake"
"include(CMakeFindDependencyMacro)
find_dependency(gdk-math 1)
include(\"\${CMAKE_CURRENT_LIST_DIR}/gdk-audio-targets.cmake\")
")

install(FILES
    "${PROJECT_BINARY_DIR}/gdk-audio-config.cmake"
    "${PROJECT_BINARY_DIR}/gdk-audio-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gdk-audio")

if (GDKAUDIO_BUILD_TESTS)
    add_subdirectory(test)
endif()

if (GDKAUDIO_BUILD_DOCS)
    add_subdirectory(docs)
endif()
