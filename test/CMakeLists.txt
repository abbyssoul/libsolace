set(TEST_SOURCE_FILES
        main_gtest.cpp
        mockTypes.cpp
        ci/teamcity_messages.cpp
        ci/teamcity_gtest.cpp

        test_atom.cpp
        test_stringView.cpp
        test_variableSpan.cpp
        test_error.cpp
        test_optional.cpp
        test_result.cpp

        test_memoryView.cpp
        test_memoryResource.cpp
        test_memoryManager.cpp

        test_array.cpp
        test_arrayView.cpp
        test_vector.cpp
        test_dictionary.cpp
        test_base16.cpp
        test_base64.cpp
        test_byteReader.cpp
        test_byteWriter.cpp
        test_uuid.cpp
        test_char.cpp
        test_string.cpp
        test_stringBuilder.cpp
        test_path.cpp
        test_env.cpp
        test_version.cpp
        test_dialstring.cpp

        hashing/test_md5.cpp
        hashing/test_murmur3.cpp
        hashing/test_sha1.cpp
        hashing/test_sha256.cpp
        )

enable_testing()

find_package(GTest REQUIRED CONFIG)

add_executable(test_${PROJECT_NAME} EXCLUDE_FROM_ALL ${TEST_SOURCE_FILES})

target_link_libraries(test_${PROJECT_NAME}
    ${PROJECT_NAME}
    $<$<NOT:$<PLATFORM_ID:Darwin>>:rt>
    GTest::gtest
    )

add_test(NAME test_${PROJECT_NAME}
    COMMAND test_${PROJECT_NAME}
    )
