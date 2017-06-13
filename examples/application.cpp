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


using namespace Solace;
using namespace Solace::Framework;


class ExampleApp : public Application {
public:

    explicit ExampleApp(const String& name) : Application(Version(1, 0, 0, "Demo")),
        _name(name)
    {}

    using Application::init;

    Result<void, Error> init(int argc, const char *argv[]) override {

        int someParam = 0;

        return CommandlineParser("Solace framework example", {
                    CommandlineParser::printHelp(),
                    CommandlineParser::printVersion("sol_example", getVersion()),
                    {0, "some-param", "Some useless parameter for the demo", &someParam},
                    {'u', "name", "Name to call", &_name}
                })
                .parse(argc, argv)
                .then([](const CommandlineParser*) { return; });
    }

    Solace::Result<int, Solace::Error> run() {
        std::cout << "Hello ";

        if (_name.empty())
            std::cout << "world";
        else
            std::cout << _name;

        std::cout << std::endl;

        return Solace::Ok<int>(EXIT_SUCCESS);
    }

private:

    Solace::String _name;
};


int main(int argc, char **argv) {

    ExampleApp app("Demo App");

    return app.init(argc, argv)
            .then([&app]() { return app.run(); })
            .orElse([](const Solace::Error& error) {
                if (error) {
                    std::cerr << "Error: " << error << std::endl;
                }

                return Ok(EXIT_FAILURE);
            })
            .unwrap();
}
