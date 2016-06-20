
find_package(PkgConfig REQUIRED)
pkg_search_module(CPPUNIT REQUIRED cppunit)

# Main lib project may disable rtti by default but cppunit requires it
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")

# Maybe someday...
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -fwhole-program")


set(TEST_SOURCE_FILES
        main.cpp
        ci/teamcity_messages.cpp
        ci/teamcity_cppunit.cpp
        test_memoryView.cpp
        test_byteBuffer.cpp
        test_optional.cpp
        test_uuid.cpp
        test_char.cpp
        test_array.cpp
        test_string.cpp
        test_stringBuilder.cpp
        test_path.cpp
        test_version.cpp
        io/test_signalDispatcher.cpp
        io/test_selector.cpp
        io/test_platformfilesystem.cpp
    )

add_executable(test_${PROJECT_NAME} ${TEST_SOURCE_FILES})
target_link_libraries(test_solace ${PROJECT_NAME} ${CPPUNIT_LIBRARIES} fmt)
