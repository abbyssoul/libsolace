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
 * An example of command line argument parser for a single action CLI.
*/

#include <solace/cli/parser.hpp>
#include <iostream>


using namespace Solace;


static constexpr StringLiteral      kAppName = "cli_single";
static const Version                kAppVersion = Version(0, 0, 1, "dev");


int main(int argc, const char **argv) {

    int intValue = 0;
    float32 floatValue = 0.0f;
    StringView userName(getenv("USER"));

    const auto res = cli::Parser("Solace cli single action example", {
                cli::Parser::printHelp(),
                cli::Parser::printVersion(kAppName, kAppVersion),

                {{"i", "intOption"}, "Some useless parameter for the demo", &intValue},
                {{"fOption"}, "Foating point value for the demo", &floatValue},
                {{"u", "name"}, "Greet user name", &userName}
            })
            .parse(argc, argv);

    if (res) {
        std::cout << "Hello '" << userName << "'" << std::endl;
        return EXIT_SUCCESS;
    } else {
        std::cerr << res.getError().toString();
        return EXIT_FAILURE;
    }
}
