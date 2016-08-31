# Build examples
# TODO(abbyssoul): Disabled as not supported by clang (without gold linker that is)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -fwhole-program")


# Serial port comunication example
set(EXAMPLE_SERIAL_SOURCE_FILES serial.cpp)
add_executable(serial ${EXAMPLE_SERIAL_SOURCE_FILES})
target_link_libraries(serial ${PROJECT_NAME})


# Application framework usage example
set(EXAMPLE_APPFRAMEWORK_SOURCE_FILES application.cpp)
add_executable(application ${EXAMPLE_APPFRAMEWORK_SOURCE_FILES})
target_link_libraries(application ${PROJECT_NAME})

