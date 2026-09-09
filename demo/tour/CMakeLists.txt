# © Joseph Cameron - All Rights Reserved

project(gdkaudio-demo)

add_executable(gdkaudio-demo
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

target_include_directories(gdkaudio-demo PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(gdkaudio-demo PRIVATE gdkaudio)

target_compile_features(gdkaudio-demo PRIVATE cxx_std_20)

