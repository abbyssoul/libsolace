
find_package(PkgConfig REQUIRED)
pkg_search_module(CPPUNIT REQUIRED cppunit)

# Main lib project may disable rtti by default but cppunit requires it
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Maybe someday...
# TODO(abbyssoul): Disabled as not supported by clang (without gold linker that is)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -fwhole-program")


set(TEST_SOURCE_FILES
        main.cpp
        mockTypes.cpp
        ci/teamcity_messages.cpp
        ci/teamcity_cppunit.cpp
        test_array.cpp
        test_arrayView.cpp
        test_memoryView.cpp
        test_memoryManager.cpp
        test_base64.cpp
        test_byteBuffer.cpp
        test_readBuffer.cpp
        test_optional.cpp
        test_result.cpp
        test_future.cpp
        test_uuid.cpp
        test_char.cpp
        test_string.cpp
        test_stringBuilder.cpp
        test_path.cpp
        test_version.cpp
        io/test_signalDispatcher.cpp
        io/test_selector_epoll.cpp
        io/test_selector_poll.cpp
        io/test_platformfilesystem.cpp
        io/test_sharedMemory.cpp
        io/test_mappedMemory.cpp
        io/test_pipe.cpp

        process/test_env.cpp
        framework/test_commandlineParser.cpp

        hashing/test_md5.cpp
        hashing/test_murmur3.cpp
        hashing/test_sha1.cpp
        hashing/test_sha256.cpp
    )

add_executable(test_${PROJECT_NAME} ${TEST_SOURCE_FILES})

if(UNIX AND NOT APPLE)
    target_link_libraries(test_${PROJECT_NAME} PRIVATE
        ${PROJECT_NAME}
        ${CPPUNIT_LIBRARIES}
        rt
        fmt-header-only)
else()
    target_link_libraries(test_${PROJECT_NAME} PRIVATE
        ${PROJECT_NAME}
        ${CPPUNIT_LIBRARIES}
        fmt-header-only)
endif()
