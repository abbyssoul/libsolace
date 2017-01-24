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


using namespace Solace::Framework;


class ExampleApp : public Application {
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

        int someParam = 0;

        return CommandlineParser("Solace framework example", {
                    CommandlineParser::printHelp(),
                    CommandlineParser::printVersion("sol_example", getVersion()),
                    { 0, "some-param", "Some useless parameter for the demo", &someParam},
                    { 'u', "name", "Name to great", &name}
                })
                .parse(argc, argv)
                .then([this](const CommandlineParser*) -> std::function<int()> {
                            return [this]() { return run(); };
                        }
                );
    }

protected:

    int run() {
        std::cout << "Hello ";

        if (name.empty())
            std::cout << "world";
        else
            std::cout << name;

        std::cout << std::endl;

        return EXIT_SUCCESS;
    }

private:

    const Solace::Version _version;
    Solace::String name;
};


int main(int argc, char **argv) {

    ExampleApp app;

    return app.init(argc, argv)
            .then([](const std::function<int()>& runner) { return runner(); })
            .orElse([](const Solace::Error& error) {
                    if (error) {
                        std::cerr << "Error: " << error << std::endl;
                    }

                    return EXIT_FAILURE;
                })
            .getResult();
}
