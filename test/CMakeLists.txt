# TODO(abbyssoul): Disabled as not supported by clang (without gold linker that is)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")

set(TEST_SOURCE_FILES
        main_gtest.cpp
        mockTypes.cpp
        ci/teamcity_messages.cpp
        ci/teamcity_gtest.cpp

        test_stringView.cpp
        test_error.cpp
        test_optional.cpp
        test_result.cpp
        test_future.cpp

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

        hashing/test_md5.cpp
        hashing/test_murmur3.cpp
        hashing/test_sha1.cpp
        hashing/test_sha256.cpp
        )

if(SOLACE_GTEST_SUPPORT)
    message(STATUS, "Using local version of GTest")

    add_subdirectory(../${SOLACE_EXTERNAL_DEP_GTEST_DIR} ${CMAKE_BINARY_DIR}/gtest EXCLUDE_FROM_ALL)
    include_directories(SYSTEM ${gtest_SOURCE_DIR}/include)
else()
    message(STATUS, "Using provided version of GTest")
endif()


enable_testing()
add_executable(test_${PROJECT_NAME} EXCLUDE_FROM_ALL ${TEST_SOURCE_FILES})
add_test(NAME test_${PROJECT_NAME} COMMAND test_${PROJECT_NAME})

target_link_libraries(test_${PROJECT_NAME} PRIVATE
    ${PROJECT_NAME}
    gtest
    $<$<NOT:$<PLATFORM_ID:Darwin>>:rt>
    )
