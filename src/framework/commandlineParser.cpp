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
#include "solace/framework/commandlineUtils.hpp"
#include "solace/parseUtils.hpp"
#include "solace/path.hpp"

#include <fmt/format.h>

#include <cstring>
#include <cstdlib>

#include <iomanip>
#include <iostream>
#include <sstream>  // std::stringstream


using namespace Solace;
using namespace Solace::Framework;


const char CommandlineParser::DefaultPrefix = '-';

Optional<Error> conversionError2(const char* fmt, const char* name, const char* value) {
    return Optional<Error>::of(fmt::format(fmt, name, value));
}



template<typename T>
Optional<Error> parseIntArgument(CommandlineParser::Context& context, T* value) {
    auto val = tryParse<T>(context.value, context.name);

    if (val) {
        *value = static_cast<T>(val.unwrap());
        return None();
    } else {
        // FIXME: Result::getError() must return Optional<Error>
        return Optional<Error>::of(val.moveError());
    }
}



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


CommandlineParser::Option::Option(char shortName, const char* longName, const char* desc, String* value) :
    Option(shortName, longName, desc, OptionArgument::Required, [value](Context& context) -> Optional<Error> {
        *value = context.value;

        return None();
    })
{
}


CommandlineParser::Option::Option(char shortName, const char* longName, const char* desc, int8* value):
    Option(shortName, longName, desc, OptionArgument::Required,
           [value](Context& context) { return parseIntArgument(context, value); })
{
}

CommandlineParser::Option::Option(char shortName, const char* longName, const char* desc, uint8* value):
    Option(shortName, longName, desc, OptionArgument::Required,
           [value](Context& context) { return parseIntArgument(context, value); })
{
}


CommandlineParser::Option::Option(char shortName, const char* longName, const char* desc, int16* value):
    Option(shortName, longName, desc, OptionArgument::Required,
           [value](Context& context) { return parseIntArgument(context, value); })
{
}

CommandlineParser::Option::Option(char shortName, const char* longName, const char* desc, uint16* value):
    Option(shortName, longName, desc, OptionArgument::Required,
           [value](Context& context) { return parseIntArgument(context, value); })
{
}


CommandlineParser::Option::Option(char shortName, const char* longName, const char* desc, int32* value):
    Option(shortName, longName, desc, OptionArgument::Required,
           [value](Context& context) { return parseIntArgument(context, value); })
{
}

CommandlineParser::Option::Option(char shortName, const char* longName, const char* desc, uint32* value):
    Option(shortName, longName, desc, OptionArgument::Required,
           [value](Context& context) { return parseIntArgument(context, value); })
{
}


CommandlineParser::Option::Option(char shortName, const char* longName, const char* desc, int64* value):
    Option(shortName, longName, desc, OptionArgument::Required,
           [value](Context& context) { return parseIntArgument(context, value); })
{
}

CommandlineParser::Option::Option(char shortName, const char* longName, const char* desc, uint64* value):
    Option(shortName, longName, desc, OptionArgument::Required,
           [value](Context& context) { return parseIntArgument(context, value); })
{
}

CommandlineParser::Option::Option(char shortName, const char* longName, const char* desc, float32* value) :
    Option(shortName, longName, desc, OptionArgument::Required,
           [value](Context& context) -> Optional<Error> {
        char* pEnd = nullptr;
        auto val = strtof(context.value, &pEnd);

        if (!pEnd || pEnd == context.value) {  // No conversion has been done
            return conversionError2("Option '{}' is not float32 value: '{}'", context.name, context.value);
        }

        *value = static_cast<float32>(val);

        return None();
    })
{
}

CommandlineParser::Option::Option(char shortName, const char* longName, const char* desc, float64* value) :
    Option(shortName, longName, desc, OptionArgument::Required
           , [value](Context& context) -> Optional<Error> {
        char* pEnd = nullptr;
        auto val = strtod(context.value, &pEnd);

        if (!pEnd || pEnd == context.value) {  // No conversion has been done
            return conversionError2("Option '{}' is not float64 value: '{}'", context.name, context.value);
        }

        *value = static_cast<float64>(val);

        return None();
    })
{
}


CommandlineParser::Option::Option(char shortName, const char* longName, const char* desc, bool* value) :
     Option(shortName, longName, desc, OptionArgument::Optional,
            [value](Context& context) {
        return (!as_boolean(context.value, value))
                ? Optional<Error>::of(fmt::format("Option '{}' is invalid boolean value: '{}'",
                                                  context.name,
                                                  context.value))  // No conversion was done!
                : None();
    })
{
}


bool CommandlineParser::Option::operator == (const CommandlineParser::Option& other) const noexcept {
    return ((_shortName == other._shortName)
            && (_longName == other._longName
                 || (_longName && other._longName && strcmp(_longName, other._longName) == 0)));
}



CommandlineParser::Argument::Argument(const char* name, const char* description, int8* value):
    Argument(name, description, [value](Context& context) { return parseIntArgument(context, value); })
{
}

// cppcheck-suppress uninitMemberVar
CommandlineParser::Argument::Argument(const char* name, const char* description, uint8* value):
    Argument(name, description, [value](Context& context) { return parseIntArgument(context, value); })
{
}

// cppcheck-suppress uninitMemberVar
CommandlineParser::Argument::Argument(const char* name, const char* description, int16* value):
    Argument(name, description, [value](Context& context) { return parseIntArgument(context, value); })
{
}

// cppcheck-suppress uninitMemberVar
CommandlineParser::Argument::Argument(const char* name, const char* description, uint16* value):
    Argument(name, description, [value](Context& context) { return parseIntArgument(context, value); })
{
}


// cppcheck-suppress uninitMemberVar
CommandlineParser::Argument::Argument(const char* name, const char* description, int32* value):
    Argument(name, description, [value](Context& context) { return parseIntArgument(context, value); })
{
}

// cppcheck-suppress uninitMemberVar
CommandlineParser::Argument::Argument(const char* name, const char* description, uint32* value):
    Argument(name, description, [value](Context& context) { return parseIntArgument(context, value); })
{
}


// cppcheck-suppress uninitMemberVar
CommandlineParser::Argument::Argument(const char* name, const char* description, int64* value):
    Argument(name, description, [value](Context& context) { return parseIntArgument(context, value); })
{
}

// cppcheck-suppress uninitMemberVar
CommandlineParser::Argument::Argument(const char* name, const char* description, uint64* value):
    Argument(name, description, [value](Context& context) { return parseIntArgument(context, value); })
{
}


// cppcheck-suppress uninitMemberVar
CommandlineParser::Argument::Argument(const char* name, const char* description, float32* value):
    Argument(name, description, [value](Context& context) {
        char* pEnd = nullptr;
        *value = strtof(context.value, &pEnd);

        return (!pEnd || pEnd == context.value)
                ? Optional<Error>::of(fmt::format("Argument '{}' invalid float32 value: '{}'",
                                                  context.name,
                                                  context.value))  // No conversion was done!
            : None();
    })
{
}


// cppcheck-suppress uninitMemberVar
CommandlineParser::Argument::Argument(const char* name, const char* description, float64* value):
    Argument(name, description, [value](Context& context) {
        char* pEnd = nullptr;
        *value = strtod(context.value, &pEnd);

        return (!pEnd || pEnd == context.value)
                ? Optional<Error>::of(fmt::format("Argument '{}' invalid float32 value: '{}'",
                                                  context.name,
                                                  context.value))  // No conversion was done!
            : None();
    })
{
}


// cppcheck-suppress uninitMemberVar
CommandlineParser::Argument::Argument(const char* name, const char* description, bool* value) :
    Argument(name, description, [value](Context& context) {
        return (as_boolean(context.value, value))
                ? Optional<Error>::of(fmt::format("Argument '{}' invalid boolean value: '{}'",
                                                  context.name,
                                                  context.value))  // No conversion was done!
            : None();
    })
{
}


// cppcheck-suppress uninitMemberVar
CommandlineParser::Argument::Argument(const char* name, const char* description, String* value) :
    Argument(name, description, [value](Context& context) {
        *value = context.value;

        return None();
    })
{
}


bool CommandlineParser::Argument::operator == (const CommandlineParser::Argument& other) const noexcept {
    return ((_description == other._description)
            || (_description && other._description && strcmp(_description, other._description) == 0));
}


CommandlineParser::CommandlineParser(const char* appDescription) :
    _prefix(DefaultPrefix),
    _description(appDescription),
    _options(),
    _arguments()
{
}


CommandlineParser::CommandlineParser(const char* appDescription,
                                     const std::initializer_list<Option>& options) :
    _prefix(DefaultPrefix),
    _description(appDescription),
    _options(options),
    _arguments()
{
}


CommandlineParser::CommandlineParser(const char* appDescription,
                                     const std::initializer_list<Option>& options,
                                     const std::initializer_list<Argument>& arguments) :
    _prefix(DefaultPrefix),
    _description(appDescription),
    _options(options),
    _arguments(arguments)
{
}


bool CommandlineParser::Option::isMatch(const char* name, char shortPrefix) const noexcept {
    if (_shortName && name) {
        if (name[0] == shortPrefix && name[1] == _shortName && name[2] == 0) {
            return true;
        }
    }

    if (_longName && name) {
        if (name[0] == shortPrefix && name[1] == shortPrefix && name[2] != 0 &&
                strcmp(name + 2, _longName) == 0)  {
            return true;
        }
    }

    return false;
}


Optional<Error>
CommandlineParser::Option::match(Context& cntx) const {
    return _callback(cntx);
}


Optional<Error>
CommandlineParser::Argument::match(Context& cntx) const {
    return _callback(cntx);
}


bool CommandlineParser::Command::isMatch(const char* arg) const noexcept {
    if (arg && _name && strcmp(arg, _name) == 0)  {
        return true;
    }

    return false;
}

Optional<Error>
CommandlineParser::Command::match(Context& cntx) const {
    return _callback(cntx);
}


template<typename... Args>
Result<const CommandlineParser*, Error>
fail(const char* msg, Args&&...args) {
    return Err(Error((fmt::format(msg, std::forward<Args>(args)...))));
}

template<typename... Args>
Result<uint, Error>
failUint(const char* msg, Args&&... args) {
    return Err(Error(fmt::format(msg, std::forward<Args>(args)...)));
}


Result<uint, Error>
parseOptions(const uint argc, const char *argv[],
             uint argcOffset,
             char prefix,
             const Array<CommandlineParser::Option>& options,
             const CommandlineParser& parser) {
    uint firstPositionalArgument = argcOffset;

    // Handle flags
    for (uint i = argcOffset; i < argc; ++i, ++firstPositionalArgument) {
        const char* arg = argv[i];

        if (arg[0] == prefix) {
            int numberMatched = 0;
            bool valueConsumed = false;

            for (auto& option : options) {

                if (option.isMatch(arg, prefix)) {
                    numberMatched += 1;

                    if (i + 1 < argc) {

                        const char* value = nullptr;
                        if (argv[i + 1][0] == prefix) {
                            if (option.getArgumentExpectations() == CommandlineParser::OptionArgument::Required) {
                                // Argument is required but none was given, error out!
                                return failUint("Option '{}' expected one argument", argv[i]);
                            } else {
                                value = "true";
                            }
                        } else {
                            value = argv[i + 1];
                            valueConsumed = true;
                        }

                        CommandlineParser::Context c {argc, argv, arg, value, parser};
                        auto r = option.match(c);
                        if (r.isSome()) {
                            return Err(r.get());
                        }

                        if (c.isStopRequired) {
                            return Err(Error("", 0));
                        }

                    } else {
                        if (option.getArgumentExpectations() == CommandlineParser::OptionArgument::Required) {
                            // Argument is required but none was given, error out!
                            return failUint("Option '{}' expected an argument and non was given.", argv[i]);
                        } else {
                            CommandlineParser::Context c {argc, argv, arg, "true", parser};
                            auto r = option.match(c);
                            if (r.isSome()) {
                                return Err(r.get());
                            }

                            if (c.isStopRequired) {
                                return Err(Error("", 0));
                            }
                        }
                    }
                }
            }

            if (numberMatched < 1) {
                return failUint("Unexpeced option '{}'", argv[i]);
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

    return Ok(firstPositionalArgument);
}


Result<const CommandlineParser*, Error>
CommandlineParser::parse(int argc, const char *argv[]) const {
    if (argc < 0)
        return fail("Negative nubmer of arguments");

    // Casting positive Int to UInt should be ok
    const uint nbOfArguments = static_cast<uint>(argc);

    auto optionsParsingResult = parseOptions(nbOfArguments, argv, 1, _prefix, _options, *this);
    if (!optionsParsingResult)
        return Err(optionsParsingResult.moveError());

    const uint firstPositionalArgument = optionsParsingResult.unwrap();

    // Positional arguments processing
    if (firstPositionalArgument >= nbOfArguments)
        return (_arguments.empty() && _commands.empty())
                ? Ok(this)
                : fail("No mandatory arguments given");

    const auto nbPositionalArguments = nbOfArguments - firstPositionalArgument;

    if (_commands.empty()) {
        if (nbPositionalArguments > _arguments.size()) {
            return fail("Too many arguments given {}, expected: {}",
                        nbPositionalArguments,
                        _arguments.size());
        }


        if (nbPositionalArguments < _arguments.size()) {
            return fail("No value given for argument {} '{}'",
                        _arguments.size() - nbPositionalArguments,
                        _arguments[_arguments.size() - nbPositionalArguments - 1].name());
        }

        for (uint i = 0; i < nbPositionalArguments; ++i) {
            Context cntx {nbOfArguments, argv,
                        _arguments[i].name(),
                        argv[firstPositionalArgument + i],
                        *this};

            _arguments[i].match(cntx);

            if (cntx.isStopRequired) {
                return Err(Error("", 0));
            }
        }
    } else {

        if (nbPositionalArguments < 1) {
            return fail("No command given");
        }

        auto positionalArgument = firstPositionalArgument;
        for (auto& cmd : _commands) {
            const char* arg = argv[positionalArgument];

            if (cmd.isMatch(arg)) {
                auto parsed = parseOptions(nbOfArguments, argv, positionalArgument + 1, _prefix, cmd.options(), *this);

                if (!parsed) {
                    return Err(parsed.moveError());
                }

                positionalArgument = parsed.unwrap();
                Context cntx {nbOfArguments, argv,
                            cmd.name(),
                            arg,
                            *this};

                cmd.match(cntx);

                if (cntx.isStopRequired) {
                    return Err(Error("", 0));
                }

            }
        }

        // All command arguments must be consumed!
        assertIndexInRange(positionalArgument, 0, nbOfArguments + 1);
        return (positionalArgument == nbOfArguments)
                ? Ok(this)
                : fail("Unexpected arguments were not processed: {}", argv[positionalArgument]);
    }

    return Ok(this);
}


Optional<Error> HelpFormatter::operator() (CommandlineParser::Context& c) {
    _output << "Usage: " << Path::parse(c.argv[0]).getBasename();
    if (!c.parser.options().empty()) {
        _output << " [options]";
    }

    if (!c.parser.commands().empty()) {
        for (const auto& arg : c.parser.commands()) {
            _output << " " << arg.name();
        }
    }
    _output << std::endl;
    _output << c.parser.description() << std::endl;

    if (!c.parser.options().empty()) {
        _output << "Options:" << std::endl;
        for (const auto& opt : c.parser.options()) {
            formatOption(c.parser.optionPrefix(), opt);
        }
    }

    if (!c.parser.commands().empty()) {
        _output << "Commands:" << std::endl;
        for (const auto& cmd : c.parser.commands()) {
            formatCommand(cmd);
        }
    }

    c.stopParsing();
    return None();
}


void HelpFormatter::formatOption(char prefixChar, const CommandlineParser::Option& option) {
    std::stringstream s;
    s << "  ";

    if (option.shortName()) {
        s << prefixChar << option.shortName();

        if (option.name()) {
            s << ", " << prefixChar << prefixChar << option.name();
        }
    } else {
        s << prefixChar << prefixChar << option.name();
    }

    _output << std::left << std::setw(26) << s.str() << option.description() << std::endl;
}


void HelpFormatter::formatCommand(const CommandlineParser::Command& cmd) {
    _output << std::left << std::setw(26)
            << "  " << cmd.name() << " - " << cmd.description() << std::endl;
}


CommandlineParser::Option
CommandlineParser::printVersion(const char* appName, const Version& appVersion) {
    return {
        'v',
        "version",
        "Print version",
        CommandlineParser::OptionArgument::NotRequired,
        VersionPrinter(appName, appVersion, std::cout)
    };
}


CommandlineParser::Option
CommandlineParser::CommandlineParser::printHelp() {
    return {
        'h',
        "help",
        "Print help",
        CommandlineParser::OptionArgument::NotRequired,
        HelpFormatter(std::cout)
    };
}
