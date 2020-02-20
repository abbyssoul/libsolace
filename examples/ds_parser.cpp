/*
*  Copyright 2018 Ivan Ryabov
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*/

#include <solace/dialstring.hpp>
#include <solace/output_utils.hpp>

#include <iostream>
#include <string.h>

using namespace Solace;


/// Print app usage
int usage(const char* progname) {
	std::cout << "Usage: " << progname
			  << "[-h] "
			  << " <STRING>..."
			  << std::endl;

	std::cout << "Parse a dial string\n\n"
			  << "Options: \n"
			  << " -h - Display help and exit\n"
			  << std::endl;

	return EXIT_SUCCESS;
}


void parseAndPrint(const char* arg) {
	auto parseResult = tryParseDailString(arg);
	if (!parseResult) {
		std::cerr << "Failed to parse \"" << arg << "\": " << parseResult.getError() << '\n';
		return;
	}

	auto const& ds = *parseResult;

	constexpr auto const N = sizeof(AtomValue);
	char protoString[N + 1];
	atomToString(ds.protocol, protoString);

	std::cout << "protocol: \"" << protoString << "\"\n"
			  << "address: \""  << ds.address << "\"\n";
	if (!ds.service.empty()) {
		std::cout << "service: \"" << ds.service << "\"\n";
	}
}

/**
 * A simple example of decoding a 9P message from a file / stdin and printing it in a human readable format.
 */
int main(int argc, char* const* argv) {
	if (argc < 2)
		return usage(argv[0]);

	if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
		return usage(argv[0]);

	for (int i = 1; i < argc; ++i) {
		parseAndPrint(argv[i]);
		if (i != argc - 1)
			std::cout << '\n';
	}

	return EXIT_SUCCESS;
}
