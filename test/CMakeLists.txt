# Maybe someday...
# TODO(abbyssoul): Disabled as not supported by clang (without gold linker that is)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")

set(TEST_SOURCE_FILES
        main_gtest.cpp
        mockTypes.cpp
        ci/teamcity_messages.cpp
        ci/teamcity_gtest.cpp

        test_memoryView.cpp
        test_memoryResource.cpp
        test_memoryManager.cpp

#        test_array.cpp
        test_arrayView.cpp
        test_vector.cpp
        test_dictionary.cpp
        test_base16.cpp
        test_base64.cpp
        test_byteReader.cpp
        test_byteWriter.cpp
        test_optional.cpp
        test_result.cpp
        test_future.cpp
        test_uuid.cpp
        test_char.cpp
        test_string.cpp
        test_stringBuilder.cpp
        test_stringView.cpp
        test_path.cpp
        test_env.cpp
        test_version.cpp

        io/test_signalDispatcher.cpp
        io/test_selector_epoll.cpp
        io/test_selector_poll.cpp
        io/test_platformfilesystem.cpp
        io/test_sharedMemory.cpp
        io/test_mappedMemory.cpp
        io/test_pipe.cpp

        hashing/test_md5.cpp
        hashing/test_murmur3.cpp
        hashing/test_sha1.cpp
        hashing/test_sha256.cpp
        )


include_directories(${gtest_SOURCE_DIR}/include ${COMMON_INCLUDES})
add_executable(test_${PROJECT_NAME} ${TEST_SOURCE_FILES})

if(UNIX AND NOT APPLE)
    target_link_libraries(test_${PROJECT_NAME} PRIVATE
        ${PROJECT_NAME}
        gtest
        rt)
else()
    target_link_libraries(test_${PROJECT_NAME} PRIVATE
        ${PROJECT_NAME}
        gtest)
endif()
