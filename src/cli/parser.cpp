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
 * @file: cli/parser.cpp
 *
 *  Created by soultaker on 18/08/16.
*******************************************************************************/

#include "solace/cli/parser.hpp"
#include "solace/cli/utils.hpp"
#include "solace/parseUtils.hpp"

#include "solace/output_utils.hpp"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <cstring>
#include <cstdlib>

#include <iomanip>
#include <iostream>
#include <sstream>  // std::stringstream

#include <utility>


using namespace Solace;
using namespace Solace::cli;


const char Parser::DefaultPrefix = '-';
const char Parser::DefaultValueSeparator = '=';



template <typename... Args>
Optional<Error>
formatOptionalError(const char* fmt, Args&&... values) {
    return Optional<Error>{in_place, fmt::format(fmt, std::forward<Args>(values)...)};
}



template<typename T>
Optional<Error>
parseIntArgument(T* dest, StringView const& value, const Parser::Context&) {
    auto val = tryParse<T>(value);

    if (val) {
        *dest = static_cast<T>(val.unwrap());
        return none;
    } else {
        // TODO(abbyssoul): Result::getError() must return Optional<Error>
        return Optional<Error>(val.moveError());
    }
}



Optional<Error>
parseBoolean(bool* dest, StringView value) {
    auto val = tryParse<bool>(value);

    if (val) {
        *dest = val.unwrap();
        return none;
    } else {
        // TODO(abbyssoul): Result::getError() must return Optional<Error>
        return Optional<Error>(val.moveError());
    }
}


Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, StringView* dest) :
    Option(names, desc, OptionArgument::Required,
           [dest](Optional<StringView> const& value, Context const&) -> Optional<Error> {
        *dest = value.get();

        return none;
    })
{
}


Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, int8* dest):
    Option(names, desc, OptionArgument::Required,
           [dest](Optional<StringView> const& value, Context const& context) {
                return parseIntArgument(dest, value.get(), context);
            })
{
}

Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, uint8* dest):
    Option(names, desc, OptionArgument::Required,
           [dest](Optional<StringView> const& value, Context const& context) {
               return parseIntArgument(dest, value.get(), context);
           })
{
}


Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, int16* dest):
    Option(names, desc, OptionArgument::Required,
           [dest](Optional<StringView> const& value, Context const& context) {
               return parseIntArgument(dest, value.get(), context);
           })
{
}

Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, uint16* dest):
    Option(names, desc, OptionArgument::Required,
           [dest](Optional<StringView> const& value, Context const& context) {
               return parseIntArgument(dest, value.get(), context);
           })
{
}


Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, int32* dest):
    Option(names, desc, OptionArgument::Required,
           [dest](Optional<StringView> const& value, Context const& context) {
               return parseIntArgument(dest, value.get(), context);
           })
{
}

Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, uint32* dest):
    Option(names, desc, OptionArgument::Required,
           [dest](Optional<StringView> const& value, Context const& context) {
               return parseIntArgument(dest, value.get(), context);
           })
{
}


Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, int64* dest):
    Option(names, desc, OptionArgument::Required,
           [dest](Optional<StringView> const& value, Context const& context) {
               return parseIntArgument(dest, value.get(), context);
           })
{
}

Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, uint64* dest):
    Option(names, desc, OptionArgument::Required,
           [dest](Optional<StringView> const& value, Context const& context) {
               return parseIntArgument(dest, value.get(), context);
           })
{
}

Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, float32* dest) :
    Option(names, desc, OptionArgument::Required,
           [dest](Optional<StringView> const& value, Context const& context) -> Optional<Error> {
        char* pEnd = nullptr;
        // FIXME(abbyssoul): not safe use of data
        auto val = strtof(value.get().data(), &pEnd);

        if (!pEnd || pEnd == value.get().data()) {  // No conversion has been done
            return formatOptionalError("Option '{}' is not float32 value: '{}'", context.name, value.get());
        }

        *dest = static_cast<float32>(val);

        return none;
    })
{
}

Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, float64* dest) :
    Option(names, desc, OptionArgument::Required,
           [dest](Optional<StringView> const& value, Context const& context) -> Optional<Error> {
        char* pEnd = nullptr;
        // FIXME(abbyssoul): not safe use of data
        auto val = strtod(value.get().data(), &pEnd);

        if (!pEnd || pEnd == value.get().data()) {  // No conversion has been done
            return formatOptionalError("Option '{}' is not float64 value: '{}'", context.name, value.get());
        }

        *dest = static_cast<float64>(val);

        return none;
    })
{
}


Parser::Option::Option(std::initializer_list<StringLiteral> names, StringLiteral desc, bool* dest) :
     Option(names, desc, OptionArgument::Optional,
            [dest](Optional<StringView> const& value, Context const&) -> Optional<Error> {
         if (value.isSome()) {
             return parseBoolean(dest, value.get());
         } else {
             *dest = true;

             return none;
         }
    })
{
}



Parser::Argument::Argument(StringLiteral name, StringLiteral description, int8* dest):
    Argument(name, description,
             [dest](StringView value, Context const& context) { return parseIntArgument(dest, value, context); })
{
}

Parser::Argument::Argument(StringLiteral name, StringLiteral description, uint8* dest):
    Argument(name, description,
             [dest](StringView value, Context const& context) { return parseIntArgument(dest, value, context); })
{
}

Parser::Argument::Argument(StringLiteral name, StringLiteral description, int16* dest):
    Argument(name, description,
             [dest](StringView value, Context const& context) { return parseIntArgument(dest, value, context); })

{
}

Parser::Argument::Argument(StringLiteral name, StringLiteral description, uint16* dest):
    Argument(name, description,
             [dest](StringView value, Context const& context) { return parseIntArgument(dest, value, context); })

{
}

Parser::Argument::Argument(StringLiteral name, StringLiteral description, int32* dest):
    Argument(name, description,
             [dest](StringView value, Context const& context) { return parseIntArgument(dest, value, context); })

{
}

Parser::Argument::Argument(StringLiteral name, StringLiteral description, uint32* dest):
    Argument(name, description,
             [dest](StringView value, Context const& context) { return parseIntArgument(dest, value, context); })

{
}

Parser::Argument::Argument(StringLiteral name, StringLiteral description, int64* dest):
    Argument(name, description,
             [dest](StringView value, Context const& context) { return parseIntArgument(dest, value, context); })

{
}

Parser::Argument::Argument(StringLiteral name, StringLiteral description, uint64* dest):
    Argument(name, description,
             [dest](StringView value, Context const& context) { return parseIntArgument(dest, value, context); })

{
}

Parser::Argument::Argument(StringLiteral name, StringLiteral description, float32* dest):
    Argument(name, description,
             [dest](StringView value, Context const& context) {
        char* pEnd = nullptr;
        // FIXME(abbyssoul): not safe use of data
        *dest = strtof(value.data(), &pEnd);

        return (!pEnd || pEnd == value.data())
                ? formatOptionalError("Argument '{}' is not float32 value: '{}'", context.name, value)
                : none;
    })
{
}


Parser::Argument::Argument(StringLiteral name, StringLiteral description, float64* dest):
    Argument(name, description,
             [dest](StringView value, Context const& context) {
        char* pEnd = nullptr;
        // FIXME(abbyssoul): not safe use of data
        *dest = strtod(value.data(), &pEnd);

        return (!pEnd || pEnd == value.data())
                ? formatOptionalError("Argument '{}' is not float64 value: '{}'", context.name, value)
                : none;
    })
{
}


Parser::Argument::Argument(StringLiteral name, StringLiteral description, bool* dest) :
    Argument(name, description, [dest](StringView value, Context const&) { return parseBoolean(dest, value); })
{
}


Parser::Argument::Argument(StringLiteral name, StringLiteral description, StringView* dest) :
    Argument(name, description, [dest](StringView value, Context const&) { *dest = value; return none; })
{
}

bool Parser::Argument::isTrailing() const noexcept {
    return name().equals("*");
}


Result<void, Error>
idleAction() {
    return Ok();
}


Parser::Parser(StringView appDescription) :
    _prefix(DefaultPrefix),
    _valueSeparator(DefaultValueSeparator),
    _defaultAction(std::move(appDescription), idleAction)
{
}


Parser::Parser(StringView appDescription, std::initializer_list<Option> options) :
    _prefix(DefaultPrefix),
    _valueSeparator(DefaultValueSeparator),
    _defaultAction(std::move(appDescription), idleAction, options)
{
}


bool Parser::Option::isMatch(StringView name) const noexcept {
    for (const auto& optName : _names) {
        if (optName == name) {
            return true;
        }
    }

    return false;
}


Optional<Error>
Parser::Option::match(Optional<StringView> const& value, Context const& cntx) const {
    return _callback(value, cntx);
}



Optional<Error>
Parser::Argument::match(StringView const& value, Context const& cntx) const {
    return _callback(value, cntx);
}



template<typename... Args>
Result<std::function<Result<void, Error> ()>, Error>
fail(const char* msg, Args&&...args) {
    return Err(Error{fmt::format(msg, std::forward<Args>(args)...), 1});
}

template<typename... Args>
Result<uint, Error>
failUint(const char* msg, Args&&... args) {
    return Err(Error{fmt::format(msg, std::forward<Args>(args)...), 1});
}


std::pair<StringView, Optional<StringView>>
parseOption(StringView arg, char prefix, char valueSeparator) {

    const StringView::size_type startIndex = (arg.substring(1).startsWith(prefix)) ? 2 : 1;
    if (startIndex >= arg.length()) {
        return std::make_pair(StringView(), none);
    }

    StringView::size_type endIndex = startIndex;

    while ((endIndex < arg.length()) && arg[endIndex] != valueSeparator) {
        ++endIndex;
    }

    if (endIndex < arg.length())
        return std::make_pair(arg.substring(startIndex, endIndex - startIndex),
                        Optional<StringView>(arg.substring(endIndex + 1)));
    else
        return std::make_pair(arg.substring(startIndex), none);
}


Result<uint, Error>
parseOptions(Parser::Context const& cntx,
             std::vector<Parser::Option> const& options,
             char prefix, char separator) {
    auto firstPositionalArgument = cntx.offset;

    // Parse array of strings until we error out or there is no more flags:
    for (uint i = firstPositionalArgument; i < cntx.argc; ++i, ++firstPositionalArgument) {
        if (!cntx.argv[i]) {
            return failUint("Invalid number of arguments!");
        }

        auto const arg = StringView{cntx.argv[i]};

        // Check if the arg string starts with a prefix char
        if (!arg.startsWith(prefix)) {  // Nope, not a flag, stop processing options
            break;
        }

        // FIXME: C++17 destructuring: auto [argName, argValue] = parseOption(...);
        auto res = parseOption(arg, prefix, separator);
        auto const argName = std::get<0>(res);
        auto argValue = Optional<StringView>{ std::move(res.second) };
        auto consumeValue = false;

        if (argValue.isNone()) {  // No argument given in --opt=value format, need to examine next argv value
            if (i + 1 < cntx.argc) {  // Check that there are more arguments in the argv, thus we can expect a value
                StringView nextArg{cntx.argv[i + 1]};

                if (!nextArg.startsWith(prefix)) {
                    argValue = std::move(nextArg);
                    consumeValue = true;
                }
            }
        }

        uint numberMatched = 0;

        auto const optCntx = Parser::Context {cntx.argc, cntx.argv, i, argName, cntx.parser};

        for (auto& option : options) {
            if (option.isMatch(argName)) {
                if (argValue.isNone() &&
                    option.getArgumentExpectations() == Parser::OptionArgument::Required) {
                    // Argument is required but none was given, error out!
                    return failUint("Option '{}' expects a value, none were given", optCntx.name);
                }

                if (consumeValue &&
                    option.getArgumentExpectations() != Parser::OptionArgument::NotRequired) {
                    consumeValue = false;
                    // Adjust current index in the array
                    ++i;
                    ++firstPositionalArgument;
                }

                numberMatched += 1;

                auto r = option.match((option.getArgumentExpectations() == Parser::OptionArgument::NotRequired)
                                                    ? none
                                                    : argValue,
                                      optCntx);
                if (r.isSome()) {
                    return Err(r.get());
                }
            }
        }

        if (numberMatched < 1) {
            return failUint("Unexpected option '{}'", argName);
        }
    }

    return Ok(firstPositionalArgument);
}


Result<uint, Error>
parseArguments(Parser::Context const& cntx,
               std::vector<Parser::Argument> const& arguments) {

    bool const expectsTrailingArgument = arguments.empty()
            ? false
            : arguments.back().isTrailing();

    auto const nbPositionalArguments = cntx.argc - cntx.offset;

    if (nbPositionalArguments < arguments.size() && !expectsTrailingArgument) {
        return failUint("Not enough arguments");
    }

    if (!expectsTrailingArgument && nbPositionalArguments > arguments.size())
        return failUint("Too many arguments");

    uint positionalArgument = cntx.offset;

    // Parse array of strings until we error out or there is no more values to consume:
    for (uint i = 0;
         i < arguments.size() && positionalArgument < cntx.argc;
         ++positionalArgument) {

        // Make sure that we didn't hit argv end:
        if (!cntx.argv[positionalArgument]) {
            return failUint("Invalid number of arguments!");
        }

        const StringView arg {cntx.argv[positionalArgument]};
        auto& targetArg = arguments[i];
        const Parser::Context subCntx{cntx.argc,
                    cntx.argv,
                    positionalArgument,
                    targetArg.name(),
                    cntx.parser};

        auto maybeError = targetArg.match(arg, subCntx);
        if (maybeError) {
            return Err(maybeError.move());
        }

        if (i + 1 < arguments.size()) {
            ++i;
        } else if (!expectsTrailingArgument) {
            ++i;
        }
    }

    return (cntx.argc == positionalArgument)
            ? Ok(positionalArgument)
            : failUint("Not enough arguments");
}


Result<Parser::ParseResult, Error>
parseCommand(Parser::Command const& cmd, Parser::Context const& cntx) {

    auto optionsParsingResult = parseOptions(cntx,
                                             cmd.options(),
                                             cntx.parser.optionPrefix(),
                                             cntx.parser.valueSeparator());
    if (!optionsParsingResult) {
        return Err(optionsParsingResult.moveError());
    }

    const uint positionalArgument = optionsParsingResult.unwrap();

    // Positional arguments processing
    if (positionalArgument < cntx.argc) {

        if (!cmd.commands().empty()) {
            auto const subcmdName = StringView {cntx.argv[positionalArgument]};
            const auto cmdIt = cmd.commands().find(subcmdName);
            if (cmdIt == cmd.commands().end()) {
                return fail("Command '{}' not supported", subcmdName);
            }

            const Parser::Context subcomandCntx{ cntx.argc,
                        cntx.argv,
                        positionalArgument + 1,
                        subcmdName,
                        cntx.parser};

            return parseCommand(cmdIt->second, subcomandCntx);
        } else if (!cmd.arguments().empty()) {
            const Parser::Context subcomandCntx{cntx.argc,
                        cntx.argv,
                        positionalArgument,
                        StringView(),
                        cntx.parser};

            auto parseResult = parseArguments(subcomandCntx, cmd.arguments());
            if (!parseResult)
                return Err(parseResult.moveError());

            return Ok<Parser::ParseResult>(cmd.action());
        } else {
            return fail("Unexpected arguments given");
        }

    } else {

        return (cmd.arguments().empty() && cmd.commands().empty())
                || (!cmd.arguments().empty() && cmd.arguments().back().isTrailing())
            ? Ok<Parser::ParseResult>(cmd.action())
            : fail("Not enough arguments");
    }
}


Result<Parser::ParseResult, Error>
Parser::parse(int argc, char const *argv[]) const {
    if (argc < 0) {
        return fail("Number of arguments can not be negative");
    }

    if (argc < 1) {
        return (_defaultAction.arguments().empty() && _defaultAction.commands().empty())
                ? Ok<Parser::ParseResult>(_defaultAction.action())
                : fail("Not enough arguments");
    }

    return parseCommand(_defaultAction, {
                            static_cast<uint>(argc),
                            argv, 1,
                            argv[0],
                            *this});
}


Parser::Option
Parser::printVersion(StringView appName, Version const& appVersion) {
    return {
        {"v", "version"},
        "Print version",
        Parser::OptionArgument::NotRequired,
        [appName, &appVersion] (Optional<StringView> const&, Context const&) -> Optional<Error> {
            VersionPrinter printer(appName, appVersion);

            printer(std::cout);

            return none;
        }
    };
}


Parser::Option
Parser::Parser::printHelp() {
    return {
        {"h", "help"},
        "Print help",
        Parser::OptionArgument::NotRequired,
        [](Optional<StringView> const& value, Context const& cntx) -> Optional<Error> {
            HelpFormatter printer(cntx.parser.optionPrefix());

            if (value.isNone()) {
                printer(std::cout,
                        cntx.argv[0],
                        cntx.parser.defaultAction());
            } else {

                const auto& cmdIt = cntx.parser.commands().find(value.get());
                if (cmdIt != cntx.parser.commands().end()) {
                    printer(std::cout,
                            cmdIt->first,
                            cmdIt->second);
                } else {
                    return Optional<Error>(Error("Unknown command"));
                }
            }

            return none;
        }
    };
}

Parser::Command::CommandDict::value_type
Parser::printVersionCmd(StringView appName, const Version& appVersion) {
    return {"version", {
            "Print version",
            [appName, appVersion]() -> Result<void, Error> {
                 VersionPrinter printer(appName, appVersion);

                 printer(std::cout);

                 return Ok();
            }
        }};
}


Parser::Command::CommandDict::value_type
Parser::printHelpCmd() {
    return {"help", {
            "Print help",
            []() -> Result<void, Error> {
                HelpFormatter printer(Parser::DefaultPrefix);
//                printer(std::cout);

                return Ok();
            }
     }};
}
