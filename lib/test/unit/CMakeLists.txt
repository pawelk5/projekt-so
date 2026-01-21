if (BUILD_TESTING)
    set(TEST_NAME "UnitTests")

    add_executable(${TEST_NAME}
        "Utils.cpp"
    )

    target_compile_features(${TEST_NAME} PRIVATE cxx_std_20)
    target_link_libraries(${TEST_NAME} PRIVATE GTest::gtest_main ${STATIC_LIBRARY})
    target_include_directories(${TEST_NAME} PRIVATE ${LIBRARY_INCLUDE_PATH})

    add_test(NAME ${TEST_NAME}
        COMMAND ${TEST_NAME}
    )

    add_custom_command(TARGET ${TEST_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${STATIC_LIBRARY}>
            $<TARGET_FILE_DIR:${TEST_NAME}>
    )

    gtest_discover_tests(${TEST_NAME})
endif()