include(CheckCXXCompilerFlag)

# Require at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# Checks for optional extra compiler options
check_cxx_compiler_flag("-fvtable-verify" WITH_VTABLE_VERIFY)
check_cxx_compiler_flag("-fstack-protector" WITH_STACK_PROTECTOR)
check_cxx_compiler_flag("-fno-omit-frame-pointer" WITH_FRAME_POINTER)
check_cxx_compiler_flag("-fsanitize-address-use-after-scope" WITH_SANITIZE_ADDRESS_USE_AFTER_SCOPE)
check_cxx_compiler_flag("-fsanitize=signed-integer-overflow" WITH_SANITIZE_SIGNED_INTEGER_OVERFLOW)
check_cxx_compiler_flag("-fsanitize=unsigned-integer-overflow" WITH_SANITIZE_UNSIGNED_INTEGER_OVERFLOW)

# Some warning are not universally supported
check_cxx_compiler_flag("-Wlogical-op" WITH_WARN_LOGICAL_OP)
check_cxx_compiler_flag("-Wstrict-null-sentinel" WITH_WARN_STRICT_NULL_SENTINEL)
check_cxx_compiler_flag("-Wimplicit-fallthrough=2" WITH_IMPLICIT_FALLTHROUGH)
check_cxx_compiler_flag("-Wnoexcept" WITH_WNOEXCEPT)


# Set compiler flags:
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -pedantic -pedantic-errors")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")


# TODO: Make this warning work too!
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsign-conversion")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wswitch-enum -Wswitch-default")

# Enable all the warnings one can get
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wmissing-include-dirs -Wpacked -Wredundant-decls")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Woverloaded-virtual")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfloat-equal")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpointer-arith -Wcast-align -Wcast-qual -Wwrite-strings")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused -Wunused-function -Wunused-label -Wunused-value -Wunused-variable")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winit-self -Wdisabled-optimization")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsign-promo -Wstrict-overflow=5")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat=2 -Wformat-security")

# -Werror
# -Wundef -Wmissing-declarations
# Not compatible wit GTest: -Wctor-dtor-privacy
# Not friendly to GTest: -Wmissing-format-attribute")
# Too many lies: set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++")

# New in gcc-7
if (WITH_IMPLICIT_FALLTHROUGH)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wimplicit-fallthrough=2")
endif(WITH_IMPLICIT_FALLTHROUGH)

# GCC specific
if (WITH_WNOEXCEPT)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnoexcept")
endif (WITH_WNOEXCEPT)
if (WITH_WARN_LOGICAL_OP)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op")
endif (WITH_WARN_LOGICAL_OP)
if (WITH_WARN_STRICT_NULL_SENTINEL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstrict-null-sentinel")
endif (WITH_WARN_STRICT_NULL_SENTINEL)


# It's better to use fortify_sources if compiler supports it
if (UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2")

    # Some extra defins for libstdc++:
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_ASSERTIONS -D_GLIBCXX_DEBUG -D_GLIBCXX_SANITIZE_VECTOR")
endif()


if (WITH_STACK_PROTECTOR)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")
endif (WITH_STACK_PROTECTOR)
# ---------------------------------
# Debug mode flags
# ---------------------------------
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb3 -D _DEBUG -D DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fstack-protector-all")

if (WITH_FRAME_POINTER)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer")
endif (WITH_FRAME_POINTER)

if (WITH_VTABLE_VERIFY)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fvtable-verify=std")
endif (WITH_VTABLE_VERIFY)


# ---------------------------------
# Release Optimization flags
# ---------------------------------
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -D NDEBUG")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND NOT "MINGW")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fuse-ld=gold")
    # Link time optimization: currently disabled as requires build system support.
#    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fuse-linker-plugin -flto")
endif()


# ---------------------------------
# When sanitizers are ON
# ---------------------------------
if (SANITIZE)
    check_cxx_compiler_flag("-fsanitize=leak" WITH_SANITIZE_LEAK)
    if (WITH_SANITIZE_LEAK)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined,leak")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
        message(STATUS, "Leak sanitizer not supported")
    endif(WITH_SANITIZE_LEAK)

    if (WITH_SANITIZE_SIGNED_INTEGER_OVERFLOW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=signed-integer-overflow")
    endif(WITH_SANITIZE_SIGNED_INTEGER_OVERFLOW)

    if (WITH_SANITIZE_UNSIGNED_INTEGER_OVERFLOW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=unsigned-integer-overflow")
    endif(WITH_SANITIZE_UNSIGNED_INTEGER_OVERFLOW)

    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    # CMake 3.13 can replace with: add_link_options(-fsanitize=address)
endif()


# ---------------------------------
# Debug build with test coverage
# ---------------------------------
if (COVERAGE)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage") # enabling coverage
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    else()
        message(FATAL_ERROR "Coverage can only be enabled in Debug mode")
    endif()
endif()


# ---------------------------------
# Include profile information
# ---------------------------------
if (PROFILE)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    else()
        message(FATAL_ERROR "Profiling requires non optimized build")
    endif()
endif()
