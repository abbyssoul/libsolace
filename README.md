# libsolace [![C++ standard][c++-standard-shield]][c++-standard-link] [![License][license-shield]][license-link]
---
[![TravisCI][travis-shield]][travis-link]
[![Codecov][codecov-shield]][codecov-link]
[![Coverity][coverity-shield]][coverity-link]
[![Coverage Status][coveralls-shield]][coveralls-link]
[![LGTM][LGTM-shield]][LGTM-link]


[c++-standard-shield]: https://img.shields.io/badge/c%2B%2B-14/17/20-blue.svg
[c++-standard-link]: https://en.wikipedia.org/wiki/C%2B%2B#Standardization
[license-shield]: https://img.shields.io/badge/License-Apache%202.0-blue.svg
[license-link]: https://opensource.org/licenses/Apache-2.0
[travis-shield]: https://travis-ci.org/abbyssoul/libsolace.png?branch=master
[travis-link]: https://travis-ci.org/abbyssoul/libsolace
[codecov-shield]: https://codecov.io/gh/abbyssoul/libsolace/branch/master/graph/badge.svg
[codecov-link]: https://codecov.io/gh/abbyssoul/libsolace
[coverity-shield]: https://scan.coverity.com/projects/9728/badge.svg
[coverity-link]: https://scan.coverity.com/projects/abbyssoul-libsolace
[coveralls-shield]: https://coveralls.io/repos/github/abbyssoul/libsolace/badge.svg?branch=master
[coveralls-link]: https://coveralls.io/github/abbyssoul/libsolace?branch=master
[LGTM-shield]: https://img.shields.io/lgtm/grade/cpp/github/abbyssoul/libsolace.svg
[LGTM-link]: https://lgtm.com/projects/g/abbyssoul/libsolace/alerts/


libSolace is a _library_ to help to build mission-critical application.
> library: a collection of types, functions, classes, etc. implementing a set of facilities (abstractions) meant to be potentially used as part of more that one program. From [Cpp Code guidelines gloassay](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#glossary)

The idea of this library is partially inspired by [NASA's Rules for Developing Safety Critical Code](http://spinroot.com/gerard/pdf/P10.pdf).
That is the aims is to provide building blocks for efficient and reliable applications using modern C++ (at least **C++17**).
_Note: it is by no means as strict implementation of all of P10 rules but an attempt to provide components that make it easy to observe these rules._

### Motivation
Solace is used to provide building primitives to develop a system of communicating processes that solve a problem via collaboration (aka cluster application/actor system).
It puts the developer in charge of the system and provides mechanisms for explicit memory management.
As such it will never spawn a thread or allocates memory after initialization.

## Contributing changes
The framework is work in progress and contributions are very welcomed.
Please see  [`CONTRIBUTING.md`](CONTRIBUTING.md) for details on how to contribute to
this project.

Please note that in order to maintain code quality a set of static code analysis tools is used as part of the build process.
Thus all contributions must be verified by these tools before PR can be accepted.


# Using the library
This library needs to be installed on your system in order to be used. There are a few ways this can be done:
 - You can install the pre-built version via [Conan](https://conan.io/) package manager. (Recommended)
 - You can build it from sources and install it locally.
 - You can install a pre-built version via your system's package manager such as deb/apt if it is available in your system's repository.

## Consuming library with Conan
The library can be added to your project dependencies with Conan:
```
[requires]
libsolace/0.1.1@abbyssoul/stable
```

While the library is not available in the Conan-central repository - you need to use:
```
    conan remote add <REMOTE> https://api.bintray.com/conan/abbyssoul/public-conan
```

Please check the latest available [binary version](https://bintray.com/abbyssoul/public-conan/libsolace%3Aabbyssoul/_latestVersion).


## Building from sources
The project build is managed via CMake with a Makefile provided to automate some common actions during the development process.

### Build tool dependencies
In order to build this project following tools must be present in the system:
* git (to check out the project and it’s external modules, see dependencies section)
* cmake (version 3.0 and above)
* doxygen (for documentation generation)
* cppcheck (static code analysis, the latest version from git is used as part of the 'codecheck' build step)
* cpplint (for static code analysis in addition to cppcheck)
* valgrind (for runtime code quality verification)

This project is using **C++17** features extensively. The minimal tested/required version of gcc is gcc-4.9.
CI is using clang-5 and gcc-7.
To install build tools on Debian based Linux distribution:
```shell
sudo apt-get update -qq
sudo apt-get install cmake doxygen python-pip valgrind ggcov
sudo pip install cpplint
```

The one external dependency is on [googletest](http://github.com/googletest/) - a unit testing framework.
It is managed as `git submodule`. Please make sure to use `git clone --recursive` when cloning the project for the first time.
You can also update an existing clone with:
```sh
git submodule update --init --recursive
```
If you have an existing checkout that you need to initialize to use submodules:
```sh
git submodule init
```

## Building the project
Current build system used for the project is `cmake`.
Please make sure that the system has `cmake` installed. Minimal confirmed version is 3.0
```sh
> cmake --version
cmake version 3.0.1
```

```shell
# In the project check-out directory:
# To build debug version with sanitizer enabled (recommended for development)
./configure --enable-debug --enable-sanitizer

# To build the library itself
make

# To build and run unit tests:
make test

# To run Valgrind on test suit:
# Please note – doesn’t work with ./configure --enable-sanitize option
make verify

# To build API documentation using doxygen:
make doc
```

To install locally for testing:
```shell
make --prefix=/user/home/<username>/test/lib install
```
To install system-wide (as root):
```shell
make install
```
To run code quality check before submission of a patch:
```shell
# Verify code quality before submission
make codecheck
```



### Target Platforms
The library is designed with the following platforms in mind:
 * [Parallella](https://www.parallella.org/)
 * [Raspberry Pi](https://www.raspberrypi.org/)
 * [BeagleBone](http://beagleboard.org/)
 * [NVIDIA Jetson](https://www.nvidia.com/en-us/autonomous-machines/embedded-systems/)
 * Linux / Freebsd / Macos


## Design
The design of the library is inspired by various functional language elements that can be found in other languages such as Rust and Java.
- Fixed-size integral types:
    - Fix sized integral types:
        - int{8,16,32}
        - uint{8,16,32}
        - float{32,64}
- OOP Memory management interface:
    - MemoryView – a OOP way of dealing with raw memory that knows its size and has associated destructor.
    - MemoryManager – a custom memory allocator that can be locked to control memory allocation after initialization.
- Immutable String (for those who are from Java land) with StringBuilder.
- Optional<> - optional monad.
- Result<> - Either monad. A good alternative to throwing an exception.
- Fixed-size collections:
    - ArrayView: a memory view as a collection of objects.
    - Array: Fixed-size array (dynamically allocated on the heap)
    - ArrayBuilder: variable-length collection
- Text utilities:
    - Formatting
    - Encoding
- Path – immutable hierarchy of string elements.
- PathBuilder – a builder object for path.



### Exceptions policy
Given the language of choice, library is designed with the idea that exceptions can be thrown and functions that don't throw are annotated accordingly. The design is the result of a few ideas:
 - Regular functions that return values but which invocations can result in an error (due to invalid internal state or invalid arguments) - this functions should signal that fact by returning Result<Value, Error> (the idea borrowed from Rust language)
 - Object constructors can't return value thus they are allowed to throw.
 - All API clients that want to use 'safe' interface should use object factories that return Result<> with possible error but don't throw.


## Dependencies
The project's external dependency managed via `git submodule`:
* [GTest](https://github.com/google/googletest) - Google's C++ test framework


## Testing
The library is equipped with the [GTest](https://github.com/google/googletest) test suit.
The source code for unit test located in directory [test](test)

Tests can (and should) be run via:
```shell
make test
```

### Developers/Contributing
Please make sure that static code check step returns no error before raising a pull request
```shell
make codecheck
```

It is also a good idea to run Valgrind on the test suit to make sure that the memory is safe
```shell
make verify
```


## Projects that might be similar in some aspects:
* [Folly](https://github.com/facebook/folly)
* [Stout](https://github.com/3rdparty/stout)
* [Poco](http://pocoproject.org/)
* [NSPR](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSPR)
* [APR](http://apr.apache.org/)
* [SDL](http://www.libsdl.org/)
* [Marmalade](http://www.madewithmarmalade.com/marmalade)
* [GLibMM](http://developer.gnome.org/glibmm/)
* [Qt(Core)](http://doc.qt.nokia.com/)


## Licensing
The library available under Apache License 2.0
Please see [`LICENSE`](LICENSE) for details.

## Authors
Please see [`AUTHORS`](AUTHORS) file for the list of contributors.
