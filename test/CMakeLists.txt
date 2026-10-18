# © Joseph Cameron - All Rights Reserved

jfc_add_tests(
    C++_STANDARD 20

    TEST_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/context_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/insert_effect_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/looping_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/scene_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/send_effect_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/emitter_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/emitter_group_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/multichannel_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/pause_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/playback_position_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/null_backend_test.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/microphone_test.cpp"

    INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_LIST_DIR}"
        "${CMAKE_CURRENT_LIST_DIR}/../src/include"

    LIBRARIES
        gdkaudio
        "${OpenAL_LIBRARIES}"
        gdkmath)

set_property(TARGET ${JFC_TEST_TARGET} PROPERTY CXX_EXTENSIONS OFF)

set_tests_properties(${JFC_TEST_TARGET} PROPERTIES ENVIRONMENT "ALSOFT_DRIVERS=null")

jfc_add_tests(
    C++_STANDARD 20

    TEST_SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/ext/vorbis_test.cpp"

    INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_LIST_DIR}"

    LIBRARIES
        gdkaudio_vorbis
        "${OpenAL_LIBRARIES}"
        gdkmath)

set_property(TARGET ${JFC_TEST_TARGET} PROPERTY CXX_EXTENSIONS OFF)

set_tests_properties(${JFC_TEST_TARGET} PROPERTIES ENVIRONMENT "ALSOFT_DRIVERS=null")
