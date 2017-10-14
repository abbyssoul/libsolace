cmake_minimum_required(VERSION 3.0)
project(solace)

include(CheckCXXCompilerFlag)


# Custom build target to generate code coverage:
# We probably don't want this to run on every build.
option(COVERALLS "Generate coveralls data" OFF)


# libsolace requires at least C++14
#set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_STANDARD_REQUIRED on)

check_cxx_compiler_flag(-std=c++14 HAVE_STD_CPP14_FLAG)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pipe -pedantic -pedantic-errors") #  -fno-rtti

# Enable stack-protector if compiler supports it (it should)
check_cxx_compiler_flag("-fstack-protector" WITH_STACK_PROTECTOR)
check_cxx_compiler_flag("-fno-omit-frame-pointer" WITH_FRAME_POINTER)

if (WITH_STACK_PROTECTOR)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
endif (WITH_STACK_PROTECTOR)



# Enable all the warnings one can get
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wmissing-include-dirs -Wpacked -Wredundant-decls")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Woverloaded-virtual")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfloat-equal")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpointer-arith -Wcast-align -Wwrite-strings")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused -Wunused-function -Wunused-label -Wunused-value -Wunused-variable")

# TODO: Make this warning work too!
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion -Wold-style-cast -Wsign-conversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsign-conversion")


# FIXME(abbyssoul): fmt produces too much noise with '-Wswitch-enum'
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat-security -Wmissing-format-attribute")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++ ")


# If the library is built as a shared object: use -fPIC
#check_cxx_compiler_flag("-fPIC" WITH_FPIC)
#if (WITH_FPIC)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
#endif (WITH_FPIC)


# It's better to use fortify_sources if compiler supports it
check_cxx_compiler_flag("-D_FORTIFY_SOURCE=2" WITH_FORTIFY_SOURCE)
if (WITH_FORTIFY_SOURCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2")
endif (WITH_FORTIFY_SOURCE)

# Optimization flags
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -O0 -ggdb3 -D _DEBUG -D DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -D NDEBUG")

if (WITH_FRAME_POINTER)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer")
endif (WITH_FRAME_POINTER)


#kernel-address
#set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined,leak")
#set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined,leak -fstack-protector-all")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

    if (COVERALLS)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage") # enabling coverage
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    endif()
else()
    if (COVERALLS)
        message(FATAL_ERROR "Coverage can only be built in Debug mode")
    endif()
endif()



# Build project dependencies (should be removed eventually)
set(FMT_USE_CPP11 OFF CACHE BOOL "Don't add std by fmt" FORCE)
add_subdirectory(external/fmt EXCLUDE_FROM_ALL)

include_directories(include)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(examples)
