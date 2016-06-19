cmake_minimum_required(VERSION 3.0)
project(solace)

include(CheckCXXCompilerFlag)

# libsolace requires c++14
check_cxx_compiler_flag(-std=c++14 HAVE_STD_CPP14_FLAG)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pedantic -pedantic-errors -fno-rtti")

# Enable stack-protector if compiler supports it (it should)
check_cxx_compiler_flag("-fstack-protector" WITH_STACK_PROTECTOR)
if (WITH_STACK_PROTECTOR)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
endif (WITH_STACK_PROTECTOR)

# Enable all worning one can wish for
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wmissing-include-dirs -Wfloat-equal -Wcast-align -Wpacked -Wredundant-decls")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpointer-arith -Wwrite-strings -Wformat-security")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused -Wunused-function -Wunused-label -Wunused-value -Wunused-variable")


# FIXME(abbyssoul): fmt produces too much noise with '-Wswitch-enum'
# add -Wconversion ?
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmissing-format-attribute")

# with -fPIC
check_cxx_compiler_flag("-fPIC" WITH_FPIC)
if (WITH_FPIC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif (WITH_FPIC)

# It's better to use fortify_sources if compiler supports it
check_cxx_compiler_flag("-D_FORTIFY_SOURCE=2" WITH_FORTIFY_SOURCE)
if (WITH_FORTIFY_SOURCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2")
endif (WITH_FORTIFY_SOURCE)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3 -O0 -D _DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -D NDEBUG") # -fwhole-program")

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
        src/io/signalDispatcher.cpp
        src/io/selector.cpp
        src/io/platformFilesystem.cpp
    )


set(EXAMPLE_SERIAL_SOURCE_FILES examples/serial.cpp)

set(FMT_USE_CPP11 OFF CACHE BOOL "Don't add std by fmt" FORCE)
add_subdirectory(libs/fmt)
add_subdirectory(test)

add_library(${PROJECT_NAME} ${SOURCE_FILES})

add_executable(serial ${EXAMPLE_SERIAL_SOURCE_FILES})

target_link_libraries(solace fmt)
target_link_libraries(serial solace)
