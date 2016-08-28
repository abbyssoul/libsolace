/*
*  Copyright 2016 Ivan Ryabov
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

#include <solace/memoryManager.hpp>
#include <solace/io/selector.hpp>
#include <solace/io/serial.hpp>
#include <solace/version.hpp>

#include <iostream>

using Solace::uint32;


void enumerateDevices() {
	for (const auto& descriptor : Solace::IO::Serial::enumeratePorts()) {
        std::cout << descriptor.file << ":" << std::endl;
        std::cout << "\t - " << descriptor.description << std::endl;
        std::cout << "\t - " << descriptor.hardwareId << std::endl;
	}
}


int main(int argc, char **argv) {
    std::cout << "libsolace: " << Solace::getBuildVersion() << std::endl;

    if (argc < 2) { // No-arg call: list ports and exit
        enumerateDevices();
		return 0; 
	}

    const uint32 boudRate = (argc > 2)
        ? atoi(argv[2])
        : 115200;

	const uint32 bufferSize = (argc > 3) 
		? atoi(argv[3])
        : 120;

	const auto file = Solace::Path::parse(argv[1]);
    std::cout << "Opening port: " << file << std::endl
              << "boudrate: " << boudRate << std::endl
              << "press ^C to quit" << std::endl;

    Solace::MemoryManager memManager(2048);

    auto buffer = memManager.create(bufferSize);
    Solace::ByteBuffer readBuffer(buffer);
	Solace::IO::Serial serial(file, boudRate);
    auto selector = Solace::IO::Selector::epoll(2);
    selector.add(&serial,   Solace::IO::Selector::Events::Read ||
                            Solace::IO::Selector::Events::Error);

    bool keepOnRunning = true;
    while (keepOnRunning) {
        for (auto event : selector.poll()) {

            if (event.events & Solace::IO::Selector::Events::Read) {
                const auto bytesRead = serial.read(readBuffer);
                if (bytesRead > 0) {
                    auto dataView = readBuffer.viewWritten();
                    std::cout.write(dataView.dataAs<const char*>(), dataView.size());
                    std::cout.flush();

                    readBuffer.rewind();
                } else {
                    std::cout << "Serial was ready but no bytes read '" << bytesRead << "'. Aborting." << std::endl;
                    keepOnRunning = false;
                }
            } else {
                std::cout << "Serial fid reported odd events: '" << event.events << "'. Aborting." << std::endl;
                keepOnRunning = false;
            }
        }
    }


    return EXIT_SUCCESS;
}
