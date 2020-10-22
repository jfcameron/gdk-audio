# © 2019 Joseph Cameron - All Rights Reserved

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/jfc-smart_handles")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/gdk-math")

set(dependencies
    "OpenAL"
)

jfc_add_dependencies(${dependencies})

