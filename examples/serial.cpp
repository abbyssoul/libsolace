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
#include <solace/exception.hpp>
#include <solace/cli/parser.hpp>


#include <iostream>



using namespace Solace;
using namespace Solace::cli;


void enumerateDevices() {
	for (const auto& descriptor : Solace::IO::Serial::enumeratePorts()) {
        std::cout << descriptor.file << ":" << std::endl;
        std::cout << "\t - " << descriptor.description << std::endl;
        std::cout << "\t - " << descriptor.hardwareId << std::endl;
	}
}


int main(int argc, const char **argv) {
    if (argc < 2) { // No-arg call: list ports and exit
        enumerateDevices();
        return 0;
	}

    uint32 boudRate = 115200;
    uint32 bufferSize = 120;
    Solace::Path file;

    const auto parseResult = Parser("Serial port example", {
                          Parser::printHelp(),
                          Parser::printVersion("serial", getBuildVersion()),
                          {{"b", "boudRate"},   "COM port boud rate",   &boudRate},
                          {{"bufferSize"},      "Read buffer size",     &bufferSize}
                        })
            .arguments({
                          { "fileName", "File/device name to open",
                            [&file](const StringView& value, const Parser::Context&) {
                                file = Solace::Path::parse(value);
                                return None();
                           }
                        }})
            .parse(argc, argv);

    if (!parseResult) {
        std::cerr << parseResult.getError() << std::endl;

        return EXIT_FAILURE;
    }

    std::cout << "Opening port: " << file << std::endl
              << "boudrate: " << boudRate << std::endl;

    try {
        Solace::IO::Serial serial(file, boudRate);

        std::cout << "press ^C to quit" << std::endl;

        Solace::MemoryManager memManager(2048);

        Solace::ByteBuffer readBuffer(memManager.create(bufferSize));
        auto selector = Solace::IO::Selector::createPoll(2);
        selector.add(&serial,   Solace::IO::Selector::Events::Read |
                                Solace::IO::Selector::Events::Error);

        bool keepOnRunning = true;
        while (keepOnRunning) {
            for (auto event : selector.poll()) {

                if ((event.events & Solace::IO::Selector::Events::Read) && (event.data == &serial)) {
                    const auto bytesRead = serial.read(readBuffer);
                    if (bytesRead) {
                        auto dataView = readBuffer.viewWritten();
                        std::cout.write(dataView.dataAs<const char>(), dataView.size());
                        std::cout.flush();

                        readBuffer.rewind();
                    } else {
                        std::cout << "Serial port signaled as ready but no bytes read: " << bytesRead.getError() << ". Aborting." << std::endl;
                        keepOnRunning = false;
                    }
                } else {
                    std::cout << "Serial port fid reported unexpected events: '" << event.events << "'. Aborting." << std::endl;
                    keepOnRunning = false;
                }
            }
        }


        return EXIT_SUCCESS;
    } catch (IOException& e) {
        std::cerr << e.toString() << std::endl;

        return EXIT_FAILURE;
    }
}
