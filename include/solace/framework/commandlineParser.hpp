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
 * libSolace: Command line parser
 *	@file		solace/framework/commandlineParser.hpp
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		Command line parser
 *	ID:			$Id: $
 ******************************************************************************/
#pragma once
#ifndef SOLACE_FRAMEWORK_COMMANDLINEPARSER_HPP
#define SOLACE_FRAMEWORK_COMMANDLINEPARSER_HPP

#include "solace/types.hpp"
#include "solace/string.hpp"
#include "solace/result.hpp"
#include "solace/unit.hpp"
#include "solace/error.hpp"
#include "solace/array.hpp"
#include "solace/version.hpp"



namespace Solace { namespace Framework {

/**
 * Comman line parser
 * This is a helper class to handle processing of command line arguments.
 *
 * @example
 * \code{.cpp}
 int main(int argc, argv) {

  CommandlineParser("My application",
            {
                // Custom handler example:
                { 'v', "version", "Print version", [this]() { return printVersionAndExit(); } },
                // Regular argument of integal type
                { 's', "size", "Buffer size", &settings.bufferSize },
                { 'u', "userName", "User name", &settings.userName }
            },
            {
                // Mandatory arguments support:
                { "Mandatory argument", &settings.param },
            })
            .verion({1, 2, 18, "Dev"})
            .parse(argc, argv)
            .then(...consume parsing results...
                  ...handle errors...);
    ...

    \endcode
 */
class CommandlineParser {
public:

    /**
     * Context used by callback function to get the state of the pasring.
     */
    struct Context {
        /// Name of the option / argument being parsed.
        const char* const name;

        /// Current option raw value.
        const char* const value;

        /// Reference to the instance of the parser that invokes the callback.
        const CommandlineParser& parser;

        bool isStopRequired;

        Context(const char* inName, const char* inValue, const CommandlineParser& self) :
            name(inName),
            value(inValue),
            parser(self),
            isStopRequired(false)
        {}

        void stopParsing() noexcept {
            isStopRequired = true;
        }
    };


    /**
     * An optional argument / flag object used by command line parser.
     */
    class Option {
    public:
        Option(char shortName, const char* longName, const char* description, int32* value);
        Option(char shortName, const char* longName, const char* description, uint32* value);
        Option(char shortName, const char* longName, const char* description, float32* value);
        Option(char shortName, const char* longName, const char* description, bool* value);
        Option(char shortName, const char* longName, const char* description, String* value);
        Option(char shortName, const char* longName, const char* description,
               const std::function<Optional<Error> (Context&)>& callback,
               bool expectsArgument = true);

        Option(const Option& rhs) noexcept :
            _shortName(rhs._shortName),
            _longName(rhs._longName),
            _description(rhs._description),
            _callback(rhs._callback),
            _expectsArgument(rhs._expectsArgument)

        {}

        Option(Option&& rhs) noexcept :
            _shortName(std::move(rhs._shortName)),
            _longName(std::move(rhs._longName)),
            _description(std::move(rhs._description)),
            _callback(std::move(rhs._callback)),
            _expectsArgument(rhs._expectsArgument)
        {}

        Option& operator= (const Option& rhs) noexcept {
            Option(rhs).swap(*this);

            return *this;
        }

        Option& operator= (Option&& rhs) noexcept {
            return swap(rhs);
        }

        Option& swap(Option& rhs) noexcept {
            std::swap(_shortName, rhs._shortName);
            std::swap(_longName, rhs._longName);
            std::swap(_description, rhs._description);
            std::swap(_callback, rhs._callback);
            std::swap(_expectsArgument, rhs._expectsArgument);

            return (*this);
        }

        bool operator == (const Option& other) const noexcept;


        bool isMatch(const char* name, char shortPrefix) const noexcept;
        bool isExpectsArgument() const noexcept { return _expectsArgument; }
        Optional<Error> match(Context& c) const;

    private:
        char                                _shortName;
        const char*                         _longName;
        const char*                         _description;
        std::function<Optional<Error> (Context&)>    _callback;

        bool                                _expectsArgument;
    };


    class Argument {
    public:
        // Mandatory positional arguments
        Argument(const char* name, const char* description, int32* value);
        Argument(const char* name, const char* description, uint32* value);
        Argument(const char* name, const char* description, float32* value);
        Argument(const char* name, const char* description, bool* value);
        Argument(const char* name, const char* description, String* value);
        Argument(const char* name, const char* description,
                 const std::function<Optional<Error> (Context&)>& callback);

        Argument(const Argument& rhs) noexcept :
            _name(rhs._name),
            _description(rhs._description),
            _callback(rhs._callback)
        {}

        Argument(Argument&& rhs) noexcept :
            _name(std::move(rhs._name)),
            _description(std::move(rhs._description)),
            _callback(std::move(rhs._callback))
        {}

        Argument& operator= (const Argument& rhs) noexcept {
            Argument(rhs).swap(*this);

            return *this;
        }

        Argument& operator= (Argument&& rhs) noexcept {
            return swap(rhs);
        }

        Argument& swap(Argument& rhs) noexcept {
            std::swap(_name, rhs._name);
            std::swap(_description, rhs._description);
            std::swap(_callback, rhs._callback);

            return (*this);
        }

        bool operator == (const Argument& other) const noexcept;

        const char* name() const noexcept {
            return _name;
        }

        Optional<Error> match(Context& c) const;

    private:
        const char*                         _name;
        const char*                         _description;
        std::function<Optional<Error>(Context&)>    _callback;
    };

public:

    static const char DefaultPrefix;

public:

    CommandlineParser(const char* appDescription, const std::initializer_list<Option>& options);
    CommandlineParser(const char* appDescription,
                      const std::initializer_list<Option>& options,
                      const std::initializer_list<Argument>& arguments);

    CommandlineParser(const CommandlineParser& rhs) :
        prefix(rhs.prefix),
        _description(rhs._description),
        _options(rhs._options),
        _arguments(rhs._arguments)
    {}

    CommandlineParser(CommandlineParser&& rhs) noexcept :
        prefix(rhs.prefix),
        _description(std::move(rhs._description)),
        _options(std::move(rhs._options)),
        _arguments(std::move(rhs._arguments))
    {}

    CommandlineParser& operator= (const CommandlineParser& rhs) noexcept {
        CommandlineParser(rhs).swap(*this);

        return *this;
    }

    CommandlineParser& operator= (CommandlineParser&& rhs) noexcept {
        return swap(rhs);
    }

    CommandlineParser& swap(CommandlineParser& rhs) noexcept {
        std::swap(prefix, rhs.prefix);
        std::swap(_description, rhs._description);
        std::swap(_options, rhs._options);
        std::swap(_arguments, rhs._arguments);

        return (*this);
    }

    Result<const CommandlineParser*, Error> parse(int argc, const char* argv[]) const;


    // TODO(abbyssoul):
    // printVersion() const
    // printUsage() const
    // printHelp() const

public:

    char prefix;

private:

    /// Human readable description of the application.
    const char*     _description;

    /// Commad line options / flags that application accepts.
    Array<Option>   _options;

    /// Mandatory arguments application requires.
    Array<Argument> _arguments;
};

}  // End of namespace Framework
}  // End of namespace Solace
#endif  // SOLACE_FRAMEWORK_COMMANDLINEPARSER_HPP
