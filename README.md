libSolace
===============
[![Build Status](https://travis-ci.org/abbyssoul/libsolace.png?branch=master)](https://travis-ci.org/abbyssoul/libsolace)
[![Coverity Scan](https://scan.coverity.com/projects/9728/badge.svg)](https://scan.coverity.com/projects/abbyssoul-libsolace)

libSolace is a simple library for building mission critical application.
Idea of this library is inspired by [NASA's Rules for Developing Safety Critical Code](http://spinroot.com/gerard/pdf/P10.pdf).
The library aims to provide building blocks for fast and reliable applications using Modern C++ dialect.
The implementation attempts to respect P10 rules to practically possible extent, but it is an ongoing effort.
Please fill free to contribute.


## Building
### Build tool dependencies
In order to build this project following tools must be present in the system:
* cmake (version 3.0 and above)
* cppunit (Unit testing framework for C++)
* doxygen (for documentation)
* cppcheck (for static code analysis, latest version from git is used as part of the 'codecheck' step)
* cpplint (for static code analysis in addition to cppcheck)
* valgrind (for runtime code verification)

The project extensively uses C++ features from C++14 standard, so the minimal required version of gcc is gcc-4.9


To install build tools on Debian based Linux distribution:
```shell
sudo apt-get update -qq
sudo apt-get install cmake doxygen python-pip valgrind ggcov libcppunit-dev
sudo pip install cpplint
```

Library also requires a few extranal dependencies that should be present in list directory:
```shell
git clone --depth 3 https://github.com/fmtlib/fmt.git libs/fmt
git clone --depth 3 https://github.com/danmar/cppcheck.git libs/cppcheck
```

It is also possible to use the script provided to download all external dependecies:
```shell
./libs/dependencies
```


## Building using CMake
Current build system used for the project is cmake. You can build the library and test using familiar cmake steps:
Please make sure that the system has cmake installed. Minimal confirmed version is 3.0
    $ cmake --version

To build the project:
```shell
$ mkdir build && cd build
$ cmake ..
$ make
```

### Using make
Makefile automating some of the common tasks is provided:
```shell
# To build the library:
make
```

To build and run unit tests:
```shell
make test
```

To build API documentation:
```shell
make doc
```

To install locally for testing (TBD, currently broken):
```shell
make install --prefix=/user/<username>/test/lib
```

To install system wide (as root)(TBD):
```
make install
```

```shell
# Verify code quality before submission
make codecheck
```


### Target Platforms
The library is designed with the following platforms in mind:
 * [Parallella](https://www.parallella.org/)
 * [Raspberry Pi](https://www.raspberrypi.org/)
 * [BeagleBone](http://beagleboard.org/)


## Design
Design of the libraty is significantly inspired by [https://github.com/3rdparty/stout](Stout), but with the focus on performance including compile time.

 - Primitives:
	- Fixed size integral types: 
		- Fix sized numeric types: 
			- int{8,16,32}
			- uint{8,16,32}
			- float{32,64}
	- Immutable String (for thouse who are from Java land)
	- Optional<>
	- Result<> (like in Rust, can't be ignored)
	- Collections (WIP):
		- Array: Fixed-size array (dynamically allocated on the heap)
		- (TBD):List/Vector: variable-length collection
		- (TBD):Dictionary: key/value hash collection
	- Exception (extending std::exception)
	- Text utilities:
		- Formatting
		- Encoding
		- String builder


### Exceptions policy
Given the language choice library is designed with the idea tha exceptions can be thrown and functions that don't throw are annotated accordingly. The design is the resutl of a few ideas:
 - Regular functions that return values but which invocations can result in an error (due to invalid internal state or invalid agruments) - this functions should signal that fact by returning Result<Value, Error> (the idea borrowed from Rust language)
 - Object constructors can't return value thus they are allowed to throw. 
 - All API clients that whant to use 'safe' interface should use object factories that return Result<> with possible error but don't throw.


## Dependencies
* [jemalloc](http://www.canonware.com/jemalloc) - General-purpose scalable concurrent malloc implementation.
* [bzip2](http://www.bzip.org/) - a high-quality data compressor.
* [libsigc++](http://libsigc.sourceforge.net/) - typesafe callback system for standard C++
* [utf8cpp](http://utfcpp.sourceforge.net/) - A simple, portable and lightweight generic library for handling UTF-8 encoded strings.
* [fmt](https://github.com/fmtlib/fmt) - C++ Format is an open-source formatting library for C++ [More info](http://fmtlib.net/latest/index.html)
* [spdlog](https://github.com/gabime/spdlog) - Very fast, header only, C++ logging library.


## Testing
The library is equiped with the [cppunit](https://sourceforge.net/projects/cppunit/) test suit.
The source code for unit test located is in:
```
./test 
```

Test can (and should) be run via:
```shell
make test
```

### Developers/Contributing
Please make sure that static code check step returns no error before raising a pull request
```shell
make codecheck
```

It is also a good idea to run valgrind on the test suit to make sure that the memory is safe
```shell
make verify
```


## Similar projects
Here are some other projects and library that can be very usefull in the development:
* [Poco](http://pocoproject.org/)
* [NSPR](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSPR)
* [ARP](http://apr.apache.org/)
* [Stout](https://github.com/3rdparty/stout)
* [SDL](http://www.libsdl.org/)
* [Qt(Core)](http://doc.qt.nokia.com/)
* [Marmalade](http://www.madewithmarmalade.com/marmalade)
* [GLibMM](http://developer.gnome.org/glibmm/)


## License 
Please see LICENSE file for details


## Authors
Please see AUTHORS file for the list of contributors

