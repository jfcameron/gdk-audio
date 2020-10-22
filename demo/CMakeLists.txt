# © 2020 Joseph Cameron - All Rights Reserved

jfc_project(executable
    NAME "gdkaudio-demo"
    VERSION 1.0
    DESCRIPTION "demo gdk-audio app"
    C++_STANDARD 17
    C_STANDARD 90

    SOURCE_LIST
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp

    PRIVATE_INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${gdkaudio_INCLUDE_DIRECTORIES}"

    LIBRARIES
        "${gdkaudio_LIBRARIES}"

    DEPENDENCIES
        "gdkaudio")

