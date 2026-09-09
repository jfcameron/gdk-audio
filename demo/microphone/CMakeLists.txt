# © Joseph Cameron - All Rights Reserved

project(gdkaudio-microphone-demo)

add_executable(gdkaudio-microphone-demo
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

target_link_libraries(gdkaudio-microphone-demo PRIVATE gdkaudio)

target_compile_features(gdkaudio-microphone-demo PRIVATE cxx_std_20)
