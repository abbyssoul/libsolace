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
/*******************************************************************************
 * @file: framework/commanalineParser.cpp
 *
 *  Created by soultaker on 18/08/16.
*******************************************************************************/

#include "solace/framework/commandlineParser.hpp"

#include <fmt/format.h>

#include <cstring>
#include <cstdlib>


using namespace Solace;
using namespace Solace::Framework;


bool as_boolean(const char* v, bool* value) {
    if (strcasecmp("true", v) == 0 || strcmp("1", v) == 0) {
        *value = true;
        return true;
    }

    if (strcasecmp("false", v) == 0 || strcmp("0", v) == 0) {
        *value = false;
        return true;
    }

    return false;
}

CommandlineParser::Option::Option(char shortName, const char* longName, const char* description, int32* value):
    _shortName(shortName),
    _longName(longName),
    _description(description),
    _callback([value](const char* v) -> Optional<Error> {
        char* pEnd = nullptr;
        *value = strtol(v, &pEnd, 0);
        return (!pEnd || pEnd == v)
                ? Optional<Error>::of(fmt::format("Can't convert '{}' to int32", v))  // No conversion was done!
                : None();
    }),

    _expectsArgument(true)
{
}

CommandlineParser::Option::Option(char shortName, const char* longName, const char* description, uint32* value):
    _shortName(shortName),
    _longName(longName),
    _description(description),
    _callback([value](const char* v) {
        char* pEnd = nullptr;
        *value = strtoul(v, &pEnd, 0);
        return (!pEnd || pEnd == v)
                ? Optional<Error>::of(fmt::format("Can't convert '{}' to uint32", v))  // No conversion was done!
                : None();
    }),
    _expectsArgument(true)
{
}


CommandlineParser::Option::Option(char shortName, const char* longName, const char* description, float32* value) :
    _shortName(shortName),
    _longName(longName),
    _description(description),
    _callback([value](const char* v) {
        char* pEnd = nullptr;
        *value = strtof(v, &pEnd);
        return (!pEnd || pEnd == v)
            ? Optional<Error>::of(fmt::format("Can't convert '{}' to float32", v))  // No conversion was done!
            : None();
    }),
    _expectsArgument(true)
{
}


CommandlineParser::Option::Option(char shortName, const char* longName, const char* description, bool* value) :
    _shortName(shortName),
    _longName(longName),
    _description(description),
    _callback([value](const char* v) {

        return (as_boolean(v, value))
                ? Optional<Error>::of(fmt::format("Can't convert '{}' to boolean", v))  // No conversion was done!
            : None();
    }),
    _expectsArgument(true)
{
}


CommandlineParser::Option::Option(char shortName, const char* longName, const char* description, String* value) :
    _shortName(shortName),
    _longName(longName),
    _description(description),
    _callback([value](const char* v) {
        *value = v;

        return None();
    }),
    _expectsArgument(true)
{
}

CommandlineParser::Option::Option(char shortName, const char* longName, const char* description,
                                  const std::function<Optional<Error> (const char*)>& callback) :
    _shortName(shortName),
    _longName(longName),
    _description(description),
    _callback(callback),
    _expectsArgument(true)
{
}


bool CommandlineParser::Option::operator == (const CommandlineParser::Option& other) const noexcept {
    return ((_shortName == other._shortName)
            && (_longName == other._longName
                 || (_longName && other._longName && strcmp(_longName, other._longName) == 0)));
}



CommandlineParser::Argument::Argument(const char* description, int32* value):
    _description(description),
    _callback([value](const char* v) {
        char* pEnd = nullptr;
        *value = strtof(v, &pEnd);
        return (!pEnd || pEnd == v)
            ? Optional<Error>::of(fmt::format("Can't convert '{}' to int32", v))  // No conversion was done!
            : None();
    })
{
}


CommandlineParser::Argument::Argument(const char* description, uint32* value):
    _description(description),
    _callback([value](const char* v) {
        char* pEnd = nullptr;
        *value = strtof(v, &pEnd);
        return (!pEnd || pEnd == v)
            ? Optional<Error>::of(fmt::format("Can't convert '{}' to uint32", v))  // No conversion was done!
            : None();
    })
{
}


CommandlineParser::Argument::Argument(const char* description, float32* value):
    _description(description),
    _callback([value](const char* v) {
        char* pEnd = nullptr;
        *value = strtof(v, &pEnd);
        return (!pEnd || pEnd == v)
            ? Optional<Error>::of(fmt::format("Can't convert '{}' to float32", v))  // No conversion was done!
            : None();
    })
{
}


CommandlineParser::Argument::Argument(const char* description, bool* value) :
    _description(description),
    _callback([value](const char* v) {
        return (as_boolean(v, value))
                ? Optional<Error>::of(fmt::format("Can't convert '{}' to boolean", v))  // No conversion was done!
            : None();
    })
{
}


CommandlineParser::Argument::Argument(const char* description, String* value) :
    _description(description),
    _callback([value](const char* v) {
        *value = v;

        return None();
    })
{
}


CommandlineParser::Argument::Argument(const char* description,
                                      const std::function<Optional<Error> (const char*)>& callback) :
    _description(description),
    _callback(callback)
{
}


bool CommandlineParser::Argument::operator == (const CommandlineParser::Argument& other) const noexcept {
    return ((_description == other._description)
            || (_description && other._description && strcmp(_description, other._description) == 0));
}


CommandlineParser::CommandlineParser(const char* appDescription,
                                     const std::initializer_list<Option>& options) :
    _description(appDescription),
    _options(options)
{
}


CommandlineParser::CommandlineParser(const char* appDescription,
                                     const std::initializer_list<Option>& options,
                                     const std::initializer_list<Argument>& arguments) :
    _description(appDescription),
    _options(options),
    _arguments(arguments)
{
}


bool CommandlineParser::Option::isMatch(const char* name, char shortPrefix, const char* longPrefix) const {

    if (!name) {  // TODO(abbyssol): Possible error case, maybe better to throw!
        return false;
    }

    if (_shortName) {
        if (name[0] == shortPrefix && name[1] == _shortName && name[2] == 0) {
            return true;
        }
    }

    if (_longName) {
        if (name[0] == longPrefix[0] && name[1] == longPrefix[1] &&
                strcmp(name + 2, _longName) == 0)  {
            return true;
        }
    }

    return false;
}


Optional<Error> CommandlineParser::Option::match(const char* value) const {
    return _callback(value);
}


Optional<Error> CommandlineParser::Argument::match(const char* value) const {
    return _callback(value);
}


Result<Unit, Error>
CommandlineParser::parse(int argc, const char *argv[]) const {

    const char optionPrefix = '-';

    int firstPositionalArgument = 1;

    // Handle flags
    for (int i = 1; i < argc; ++i, ++firstPositionalArgument) {
        if (argv[i][0] == optionPrefix) {

            int numberMatched = 0;
            bool valueConsumed = false;
            for (auto& option : _options) {

                if (option.isMatch(argv[i], optionPrefix, "--")) {
                    numberMatched += 1;
                    if (i + 1 < argc) {
                        auto r = option.match(argv[i + 1]);
                        if (r.isSome()) {
                            return Err<Unit, Error>(std::move(r.get()));
                        }

                        valueConsumed = true;
                    } else {
                        if (option.isExpectsArgument()) {
                            return Err<Unit, Error>(Error(
                            fmt::format("Option '{}' expected an argument and non was given.", argv[i])));
                        }  // Else - moving on
                    }
                }
            }

            if (numberMatched < 1) {
                return Err<Unit, Error>(Error(fmt::format("Unexpeced option '{}'", argv[i])));
            }

            if (valueConsumed) {
                ++i;
                ++firstPositionalArgument;
            }

        } else {
            // Stop processing flags
            break;
        }
    }

    if (firstPositionalArgument >= argc && !_arguments.empty()) {
        return Err<Unit, Error>(Error(fmt::format("Expecting argument '1'")));
    } else {
        for (int i = firstPositionalArgument; i < argc; ++i) {
//            if (argv[i][0] == optionPrefix) {
//                return Err<Unit, Error>({fmt::format("Unexpected argument '1'")});
//            }


            _arguments[firstPositionalArgument - i].match(argv[i]);
        }
    }


    return Ok<Unit, Error>(Unit());
}
