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

/**
 * An example of Using application framework and command line argument handling.
 *
 */

#include <solace/framework/application.hpp>
#include <solace/framework/commandlineParser.hpp>


#include <iostream>


class ExampleApp : public Solace::Framework::Application {
public:
    ExampleApp() :
        _version(1, 0, 0, "Demo")
    {}

    Solace::Version getVersion() const noexcept override {
        return _version;
    }

    using Application::init;

    Solace::Result<std::function<int()>, Solace::Error>
    init(int argc, const char *argv[]) override {

        bool isVersionRequested = false;

        return Solace::Framework::CommandlineParser("Solace framework example", {
                    { 'v', "version", "Print application version.", &isVersionRequested}
                })
                .parse(argc, argv)
                .then<Solace::Result<std::function<int()>, Solace::Error>>(
                        [this, isVersionRequested](Solace::Unit) {
                            return isVersionRequested
                                        ? Solace::Ok<std::function<int()>, Solace::Error>( [this]() { return run(); })
                                        : Solace::Ok<std::function<int()>, Solace::Error>( [this]() { return printVersion(); });
                        },
                        [this](Solace::Error e) {
                            return Solace::Err<std::function<int()>, Solace::Error>(std::move(e));
                    });

    }

protected:

    int printVersion() {
        std::cout << "Version: " << getVersion() << std::endl;

        return EXIT_SUCCESS;
    }

    int run() {
        std::cout << "Hello world" << std::endl;

        return EXIT_SUCCESS;
    }

private:

    const Solace::Version _version;
};


int main(int argc, char **argv) {

    ExampleApp app;

    return app.init(argc, argv)
            .then<int>(
                [](const std::function<int()>& runner) { return runner(); },
                [](const Solace::Error& error) {
                    if (error) {
                        std::cerr << "Error: " << error << std::endl;
                    }

                    return EXIT_FAILURE;
                });
}
