cmake_minimum_required(VERSION 3.0)
project(solace)

include(CheckCXXCompilerFlag)


# Custom build target to generate code coverage:
# We probably don't want this to run on every build.
option(COVERALLS "Generate coveralls data" OFF)
option(COVERALLS_UPLOAD "Upload the generated coveralls json" ON)


# libsolace requires at least C++14
check_cxx_compiler_flag(-std=c++14 HAVE_STD_CPP14_FLAG)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pipe -pedantic -pedantic-errors") #  -fno-rtti

# Enable stack-protector if compiler supports it (it should)
check_cxx_compiler_flag("-fstack-protector" WITH_STACK_PROTECTOR)
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
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -O0 -D _DEBUG -ggdb3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -D NDEBUG") # -fwhole-program")


#kernel-address
#set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined,leak -fstack-protector-all")
#set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined,leak -fstack-protector-all")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")


if (COVERALLS)
    include(Coveralls)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0") # debug, no optimisation
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage") # enabling coverage
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")

    set(COVERAGE_SRCS awesome.c code.c files.c)

    # Create the coveralls target.
    coveralls_setup(
        "${SOURCE_FILES}" # The source files.
        ON                 # If we should upload.
        "${PROJECT_SOURCE_DIR}/CMakeModules/") # (Optional) Alternate project cmake module path.
endif()


# Build project dependencies (should be removed eventually)
set(FMT_USE_CPP11 OFF CACHE BOOL "Don't add std by fmt" FORCE)
add_subdirectory(libs/fmt)


include_directories(include)

add_subdirectory(libsolace)
add_subdirectory(test)
add_subdirectory(examples)
