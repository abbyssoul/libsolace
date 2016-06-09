
#include <solace/char.hpp>
#include <solace/stringBuilder.hpp>
#include <solace/version.hpp>
#include <solace/io/serial.hpp>
#include <solace/io/selector.hpp>


#include <iostream>

using Solace::uint32;

void enumeratePorts() {
	for (const auto& descriptor : Solace::IO::Serial::enumeratePorts()) {
        std::cout << descriptor.file << ":" << std::endl;
        std::cout << "\t - " << descriptor.description << std::endl;
        std::cout << "\t - " << descriptor.hardwareId << std::endl;
	}
}


int main(int argc, char **argv) {
    std::cout << "libsolace: " << Solace::getBuildVersion() << std::endl;

	if (argc == 1) { // No-arg call: list ports and exit
		enumeratePorts();
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

    Solace::ByteBuffer readBuffer(bufferSize);
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
                    std::cout.write((const char*)readBuffer.data(), readBuffer.position());
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
}
