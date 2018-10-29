cmake_minimum_required(VERSION 3.3)
project(solace)

include(CheckCXXCompilerFlag)


# Custom build target to generate code coverage:
# We probably don't want this to run on every build.
option(COVERALLS "Generate coveralls data" OFF)
option(SANITIZE "Enable 'sanitize' compiler flag" OFF)


# libsolace requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -pedantic -pedantic-errors") #  -fno-rtti

# Enable stack-protector if compiler supports it (it should)
check_cxx_compiler_flag("-fvtable-verify" WITH_VTABLE_VERIFY)
check_cxx_compiler_flag("-Wimplicit-fallthrough=2" WITH_IMPLICIT_FALLTHROUGH)
#check_cxx_compiler_flag("-Wl,-fuse-ld=gold" WITH_GOLD_LINKER)
check_cxx_compiler_flag("-fstack-protector" WITH_STACK_PROTECTOR)
check_cxx_compiler_flag("-fno-omit-frame-pointer" WITH_FRAME_POINTER)
check_cxx_compiler_flag("-fsanitize-address-use-after-scope" WITH_SANITIZE_ADDRESS_USE_AFTER_SCOPE)
check_cxx_compiler_flag("-fsanitize=signed-integer-overflow" WITH_SANITIZE_SIGNED_INTEGER_OVERFLOW)
check_cxx_compiler_flag("-fsanitize=unsigned-integer-overflow" WITH_SANITIZE_UNSIGNED_INTEGER_OVERFLOW)


# Enable all the warnings one can get
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wmissing-include-dirs -Wpacked -Wredundant-decls")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Woverloaded-virtual -Wnoexcept")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfloat-equal")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpointer-arith -Wcast-align -Wwrite-strings")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused -Wunused-function -Wunused-label -Wunused-value -Wunused-variable")

# New in gcc-7
if (WITH_IMPLICIT_FALLTHROUGH)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wimplicit-fallthrough=2")
endif(WITH_IMPLICIT_FALLTHROUGH)

# TODO: Make this warning work too!
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion -Wold-style-cast -Wsign-conversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsign-conversion")


#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wswitch-enum")
# Not friendly to GTest: -Wmissing-format-attribute")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat-security")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++ ")


# It's better to use fortify_sources if compiler supports it
check_cxx_compiler_flag("-D_FORTIFY_SOURCE=2" WITH_FORTIFY_SOURCE)
if (WITH_FORTIFY_SOURCE AND UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2")

    # Some extra defins for libstdc++:
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_ASSERTIONS -D_GLIBCXX_DEBUG -D_GLIBCXX_SANITIZE_VECTOR")
endif()


if (WITH_STACK_PROTECTOR)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")
endif (WITH_STACK_PROTECTOR)


# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb3 -D _DEBUG -D DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -fstack-protector-all")

if (WITH_FRAME_POINTER)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer")
endif (WITH_FRAME_POINTER)

if (WITH_VTABLE_VERIFY)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fvtable-verify=std")
endif (WITH_VTABLE_VERIFY)


# Release Optimization flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -D NDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -mtune=native")

# Extra optimizations on GCC
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND NOT MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fuse-ld=gold")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fuse-linker-plugin -flto-report -flto")

    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
endif()


# When sanitizers are ON
if (SANITIZE)
    check_cxx_compiler_flag("-fsanitize=leak" WITH_SANITIZE_LEAK)
    if (WITH_SANITIZE_LEAK)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined,leak")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
        message(STATUS, "Leak sanitizer not supported")
    endif(WITH_SANITIZE_LEAK)

    if (WITH_SANITIZE_ADDRESS_USE_AFTER_SCOPE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize-address-use-after-scope")
    endif(WITH_SANITIZE_ADDRESS_USE_AFTER_SCOPE)

    if (WITH_SANITIZE_SIGNED_INTEGER_OVERFLOW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=signed-integer-overflow")
    endif(WITH_SANITIZE_SIGNED_INTEGER_OVERFLOW)

    if (WITH_SANITIZE_UNSIGNED_INTEGER_OVERFLOW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=unsigned-integer-overflow")
    endif(WITH_SANITIZE_UNSIGNED_INTEGER_OVERFLOW)


    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()


if (COVERALLS)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage") # enabling coverage
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    else()
        message(FATAL_ERROR "Coverage can only be built in Debug mode")
    endif()
endif()



# Build project dependencies
add_subdirectory(external/gtest/googletest EXCLUDE_FROM_ALL)

include_directories(include)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(examples)

message(STATUS, "BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS, "CXXFLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS, "SANITIZE: ${SANITIZE}")
message(STATUS, "COVERALLS: ${COVERALLS}")

