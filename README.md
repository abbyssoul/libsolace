libSolace
===============
[![Build Status](https://travis-ci.org/abbyssoul/libsolace.png?branch=master)](https://travis-ci.org/abbyssoul/libsolace)

libSolace is a simple library for mission critical application.
Idea of the library is inspired by [http://spinroot.com/gerard/pdf/P10.pdf NASA's Rules for Developing Safety Critical Code].
The library aims to provide building blocks for fast and reliable applications using Modern C++.
The implementation attempts to respect P10 rules to practically possible extent, but it is an ongoing effort.
Please fill free to contribute.


== Building ==
=== Build dependencies ===
* cmake
* cppunit
* doxygen (for documentation)
* cppcheck (for static code analysis)
* cpplint (for static code analysis)
* valgrind

On linux:
```shell
sudo apt-get install cmake doxygen python-pip cppcheck valgrind ggcov
sudo apt-get install libcppunit-dev libcppunit-1.12-1
sudo pip install cpplint
```

=== Using cmake ===
Current build system used for the project is cmake. You can build the library and test using familiar cmake steps:
Please make sure that the system has cmake installed. Minimal confirmed version is 3.2
    $ cmake --version

To build the project:
    $ mkdir build && cd build
    $ cmake ..
    $ make

=== Using make ===
For convenience hand crafted makefile is provided to automake project management: 
To build a shared library:
	./configure.sh
	make

To build and run test suit:
	make test

To build API documentation:
	make doc

To install locally for testing (TBD):
	make install --prefix=/user/<username>/test/lib

To install system wide (as root)(TBD):
	make install


=== Target Systems ===
The library is designed with the following systems in mind:
 * [https://www.raspberrypi.org/ Raspberry Pi]
 * [http://beagleboard.org/ BeagleBone]
 * [https://www.parallella.org/ Parallella])

As a result of this - GUI components are not included.


== Design ==
Library design is significantly inspired by [https://github.com/3rdparty/stout](Stout) but with the focus on performance including compile time.

 - Primitives:
	- Fixed size integral types: 
		- Fix sized numeric types: 
			- int{8,16,32}
			- uint{8,16,32}
			- float{32,64}
	- Immutable String (for thouse who are from java land)
	- Optional<>
	- Result<>
	- Collections (WIP):
		- Array: Fixed-size array (dynamically allocated on the heap)
		- (TBD):List/Vector: variable-length collection
		- (TBD):Dictionary: key/value hash collection
	- Exception (extending std::exception)
	- Text utilities:
		- Formatting
		- Encoding
		- String builder

=== Exception policy ===
Given the language choice library is designed with the idea tha exceptions can be thrown and functions that don't throw are annotated accordingly. The design is the resutl of a few ideas:
 - Regular functions that return values but which invocations can result in an error (due to invalid internal state or invalid agruments) - this functions should signal that fact by returning Result<Value, Error> (the idea borrowed from Rust language)
 - Object constructors can't return value thus they are allowed to throw. 
 - All API clients that whant to use 'safe' interface should use object factories that return Result<> with possible error but don't throw.


== Dependencies ==
* [http://www.canonware.com/jemalloc](jemalloc) - General-purpose scalable concurrent malloc implementation.
* [http://www.bzip.org/](bzip2) - a high-quality data compressor.
* [http://libsigc.sourceforge.net/](libsigc++) - typesafe callback system for standard C++
* [http://utfcpp.sourceforge.net/](utf8cpp) - A simple, portable and lightweight generic library for handling UTF-8 encoded strings.
* [https://github.com/cppformat/cppformat](cppformat) - C++ Format is an open-source formatting library for C++
[http://cppformat.github.io/latest/index.html](More info)
* [https://github.com/gabime/spdlog](spdlog) - Very fast, header only, C++ logging library.

=== Testing ===
Test suit is based on [https://sourceforge.net/projects/cppunit/](cppunit)
Test suit source code is in:
./test 

Test can be run via:
make test


== Similar projects ==
* [http://pocoproject.org/](Poco)
* [https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSPR](NSPR)
* [http://apr.apache.org/](ARP)
* [https://github.com/3rdparty/stout](Stout)
* [http://www.libsdl.org/](SDL)
* [http://doc.qt.nokia.com/](Qt(Core))
* [http://www.madewithmarmalade.com/marmalade](Marmalade)
* [http://developer.gnome.org/glibmm/](GLibMM)


= License =
Please see LICENSE file for details


= Authors =
Please see AUTHORS file for the list of contributors
