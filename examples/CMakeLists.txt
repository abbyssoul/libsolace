# Build examples
# TODO(abbyssoul): Disabled as not supported by clang (without gold linker that is)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -fwhole-program")


# CLI single command flavour:
set(EXAMPLE_APPFRAMEWORK_SOURCE_FILES cli_single.cpp)
add_executable(cli_single ${EXAMPLE_APPFRAMEWORK_SOURCE_FILES})
target_link_libraries(cli_single ${PROJECT_NAME})

# CLI multi-command flavour:
set(EXAMPLE_APPFRAMEWORK_SOURCE_FILES cli_multi.cpp)
add_executable(cli_multi ${EXAMPLE_APPFRAMEWORK_SOURCE_FILES})
target_link_libraries(cli_multi ${PROJECT_NAME})


# Serial port communication example
set(EXAMPLE_SERIAL_SOURCE_FILES serial.cpp)
add_executable(serial ${EXAMPLE_SERIAL_SOURCE_FILES})
target_link_libraries(serial ${PROJECT_NAME})
