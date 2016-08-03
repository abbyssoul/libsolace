cmake_minimum_required(VERSION 3.0)
project(solace)

include(CheckCXXCompilerFlag)

# libsolace requires c++14
check_cxx_compiler_flag(-std=c++14 HAVE_STD_CPP14_FLAG)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pipe -pedantic -pedantic-errors") #  -fno-rtti

# Enable stack-protector if compiler supports it (it should)
check_cxx_compiler_flag("-fstack-protector" WITH_STACK_PROTECTOR)
if (WITH_STACK_PROTECTOR)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
endif (WITH_STACK_PROTECTOR)

# Enable all worning one can wish for
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wmissing-include-dirs -Wpacked -Wredundant-decls")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Woverloaded-virtual")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfloat-equal")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpointer-arith -Wcast-align -Wwrite-strings")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused -Wunused-function -Wunused-label -Wunused-value -Wunused-variable")

# TODO: Make it work!
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion -Wold-style-cast -Wsign-conversion")

# FIXME(abbyssoul): fmt produces too much noise with '-Wswitch-enum'
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat-security -Wmissing-format-attribute")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++ ")


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

# Optimization flags
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -O0 -D _DEBUG -ggdb3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -D NDEBUG") # -fwhole-program")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")


include_directories(include)



set(EXAMPLE_SERIAL_SOURCE_FILES examples/serial.cpp)

set(FMT_USE_CPP11 OFF CACHE BOOL "Don't add std by fmt" FORCE)
add_subdirectory(libs/fmt)
add_subdirectory(libsolace)
add_subdirectory(test)

add_executable(serial ${EXAMPLE_SERIAL_SOURCE_FILES})

target_link_libraries(serial solace)
