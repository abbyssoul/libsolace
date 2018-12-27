cmake_minimum_required(VERSION 3.3)
project(solace)


# Custom build target to generate code coverage:
# Don't want this to run on every build.
option(SOLACE_GTEST_SUPPORT "Build without GTEST" ON)
option(SOLACE_COVERALLS "Generate coveralls data" OFF)
option(SOLACE_SANITIZE "Enable 'sanitize' compiler flag" OFF)


# Include common compile flag
include(cmake/compile_flags.cmake)

# libsolace requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# ---------------------------------
# Build project dependencies
# ---------------------------------
set(SOLACE_EXTERNAL_DEP_GTEST_DIR "external/gtest/googletest"
    CACHE PATH "The path to the Google Test framework.")

include_directories(include)

add_subdirectory(src)
add_subdirectory(test EXCLUDE_FROM_ALL)
add_subdirectory(examples EXCLUDE_FROM_ALL)

message(STATUS, "BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS, "CXXFLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS, "SANITIZE: ${SANITIZE}")
message(STATUS, "COVERALLS: ${COVERALLS}")
message(STATUS, "SOLACE_GTEST_SUPPORT: ${SOLACE_GTEST_SUPPORT}")
