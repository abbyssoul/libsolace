cmake_minimum_required(VERSION 3.2)
project(libsolace)

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -O0 -D _DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -D NDEBUG -combine -fwhole-program")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -pedantic -Wextra -pedantic-errors")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -pedantic -Wextra -pedantic-errors")

# FIXME(abbyssoul): fmt produces too much noise with '-Wswitch-enum'
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow -Wmissing-include-dirs -Wfloat-equal -Wcast-align -Wpacked -Wredundant-decls")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")


include_directories(include)

set(SOURCE_FILES
        src/byteBuffer.cpp 
        src/char.cpp
        src/uuid.cpp
        src/string.cpp
        src/stringBuilder.cpp
        src/exception.cpp
        src/version.cpp
        src/path.cpp 
        
        src/io/ioexception.cpp 
        src/io/file.cpp 
        src/io/serial.cpp
        src/io/serial_list_ports.cpp
	)

set(TEST_SOURCE_FILES
        test/main.cpp
        test/ci/teamcity_messages.cpp
        test/ci/teamcity_cppunit.cpp
        test/test_byteBuffer.cpp
        test/test_optional.cpp
        test/test_uuid.cpp
        test/test_char.cpp
        test/test_array.cpp
        test/test_string.cpp
        test/test_stringBuilder.cpp
        test/test_path.cpp
        test/test_version.cpp
         )

set(EXAMPLE_SERIAL_SOURCE_FILES examples/serial.cpp)

set(FMT_USE_CPP11 OFF)
add_subdirectory(libs/fmt)

add_library(solace ${SOURCE_FILES})
add_executable(test_solace ${TEST_SOURCE_FILES})
add_executable(serial ${EXAMPLE_SERIAL_SOURCE_FILES})

target_link_libraries(solace fmt)
target_link_libraries(test_solace cppunit solace fmt)
target_link_libraries(serial solace)
