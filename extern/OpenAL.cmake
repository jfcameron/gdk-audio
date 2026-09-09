# © Joseph Cameron - All Rights Reserved

if (CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(LIBTYPE STATIC) 

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

