# Build examples
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -fwhole-program")


# Serial port comunication example
set(EXAMPLE_SERIAL_SOURCE_FILES serial.cpp)
add_executable(serial ${EXAMPLE_SERIAL_SOURCE_FILES})
target_link_libraries(serial ${PROJECT_NAME})
