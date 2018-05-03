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
 *	@file		solace/cli/parser.hpp
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		Commandline arguments Parser
 *	ID:			$Id: $
 ******************************************************************************/
#pragma once
#ifndef SOLACE_CLI_PARSER_HPP
#define SOLACE_CLI_PARSER_HPP

#include "solace/stringView.hpp"
#include "solace/result.hpp"
#include "solace/error.hpp"
#include "solace/array.hpp"
#include "solace/version.hpp"

// TODO(abbyssoul): consider moving away from std::function #include "solace/delegate.hpp"
#include "solace/utils.hpp"

#include <map>  // TODO(abbyssoul): Replace with fix-memory map

namespace Solace { namespace cli {

/**
 * Command line parser
 * This is a helper class to handle processing of command line arguments.
 *
 * @example
 * \code{.cpp}
 int main(int argc, argv) {

  cli::Parser("My application", {
                // Custom handler example:
                CommandlineParser::printVersion("my_app", Version(1, 2, 3, "dev")),
                CommandlineParser::printHelp(),

                // Regular options of integral type
                {{"size"}, "Buffer size", &settings.bufferSize },
                {{"u", "userName"}, "User name", &settings.userName }
            })
            .commands({
                    {"doSomething", { // Commands support optional arguments:
                        { "Mandatory argument", &settings.param },
                        [] () {   // Action to execute.
                            std::cout << "Executing command" << std:endl;
                        }
                    }
            }})
            .parse(argc, argv)
            .then(...consume parsing results...
            .orElse(...handle errors...);
    ...

    \endcode


 * Note: Command line parsing is meant to be performed at the start of an application.
 * In that case memory allocation limits / strategy can be configured via options.
 * That is why it is desirable that parser don't allocate any memory during parsing.
 * To support that - parser work with StringView and StringLiteral types that don't take ownership of the string buffer.
 */
class Parser {
public:

    /**
     * Parser context.
     * This object represents the current state of parsing.
     * It is designed to be used by a callback function to get access to the parameters and parser object itself.
     * It also can be used to communicate back to the parser if an interruption is required.
     */
    struct Context {

        /// Initial number of arguments passed to the 'parse' method.
        const uint argc;

        /// Individual command line arguments the parse method has been given.
        const char** argv;

        /// Current parser offset into argv.
        const uint offset;

        /// Name of the option / argument being parsed.
        const StringView name;

        /// Reference to the instance of the parser that invokes the callback.
        const Parser& parser;

        Context(uint inArgc, const char *inArgv[], uint inOffset,
                StringView inName,
                const Parser& self) :
            argc(inArgc),
            argv(inArgv),
            offset(inOffset),
            name(inName),
            parser(self)
        {}

    };

    /**
     * Argument processing policy for custom callbacks
     */
    enum class OptionArgument {
        Required,          //!< Argument is required. It is an error if the option is given without an value.
        Optional,          //!< Argument is optional. It is not an error to have option with or without an argument.
        NotRequired        //!< Argument is not expected. It is an error to give an option with an argument value.
    };


    /**
     * An optional argument / flag object used by command line parser.
     */
    class Option {
    public:
        Option(std::initializer_list<StringLiteral> names, StringLiteral description, StringView* value);
        Option(std::initializer_list<StringLiteral> names, StringLiteral description, int8* value);
        Option(std::initializer_list<StringLiteral> names, StringLiteral description, uint8* value);
        Option(std::initializer_list<StringLiteral> names, StringLiteral description, int16* value);
        Option(std::initializer_list<StringLiteral> names, StringLiteral description, uint16* value);
        Option(std::initializer_list<StringLiteral> names, StringLiteral description, int32* value);
        Option(std::initializer_list<StringLiteral> names, StringLiteral description, uint32* value);
        Option(std::initializer_list<StringLiteral> names, StringLiteral description, int64* value);
        Option(std::initializer_list<StringLiteral> names, StringLiteral description, uint64* value);
        Option(std::initializer_list<StringLiteral> names, StringLiteral description, float32* value);
        Option(std::initializer_list<StringLiteral> names, StringLiteral description, float64* value);
        Option(std::initializer_list<StringLiteral> names, StringLiteral description, bool* value);

        /// Common constructor:
        template<typename F>
        Option(std::initializer_list<StringLiteral> names,
               StringLiteral description,
               OptionArgument expectsArgument,
               F&& f) :
            _names(names),
            _description(std::move(description)),
            _expectsArgument(expectsArgument),
            _callback(std::forward<F>(f))
        {}

        Option(const Option& rhs) = default;

        Option(Option&& rhs) noexcept = default;

        Option& operator= (const Option& rhs) noexcept {
            Option(rhs).swap(*this);

            return *this;
        }

        Option& operator= (Option&& rhs) noexcept {
            return swap(rhs);
        }

        Option& swap(Option& rhs) noexcept {
            using std::swap;
            swap(_names, rhs._names);
            swap(_description, rhs._description);
            swap(_callback, rhs._callback);
            swap(_expectsArgument, rhs._expectsArgument);

            return (*this);
        }

        bool isMatch(StringView argName) const noexcept;

        Optional<Error> match(const Optional<StringView>& value, const Context& c) const;

        const Array<StringLiteral>& names() const noexcept      { return _names; }
        const StringLiteral& description() const noexcept       { return _description; }

        OptionArgument getArgumentExpectations() const noexcept { return _expectsArgument; }

    private:
        //!< Long name of the option, Maybe empty if not specified.
        Array<StringLiteral>                _names;

        //!< Human-readable description of the option.
        StringLiteral                       _description;

        //!< Enum to indicate if this option expects a value or not.
        OptionArgument                      _expectsArgument;

        //!< A callback to be called when this option is encountered in the input cmd line.
        std::function<Optional<Error> (const Optional<StringView>&, const Context&)>    _callback;
    };


    /** Mandatory argument
     * This class represent a mandatory argument to be expected by a parser.
     * It is a parsing error if no mandatory arguments is provided.
     */
    class Argument {
    public:
        Argument(StringLiteral name, StringLiteral description, StringView* value);
        Argument(StringLiteral name, StringLiteral description, int8* value);
        Argument(StringLiteral name, StringLiteral description, uint8* value);
        Argument(StringLiteral name, StringLiteral description, int16* value);
        Argument(StringLiteral name, StringLiteral description, uint16* value);
        Argument(StringLiteral name, StringLiteral description, int32* value);
        Argument(StringLiteral name, StringLiteral description, uint32* value);
        Argument(StringLiteral name, StringLiteral description, int64* value);
        Argument(StringLiteral name, StringLiteral description, uint64* value);
        Argument(StringLiteral name, StringLiteral description, float32* value);
        Argument(StringLiteral name, StringLiteral description, float64* value);
        Argument(StringLiteral name, StringLiteral description, bool* value);

        template<typename F>
        Argument(StringLiteral name, StringLiteral description,
                 F&& callback) :
            _name(std::move(name)),
            _description(std::move(description)),
            _callback(std::forward<F>(callback))
        {}

        Argument(const Argument& rhs) = default;

        Argument(Argument&& rhs) noexcept = default;

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

        StringLiteral name() const noexcept {
            return _name;
        }

        Optional<Error> match(const StringView& value, const Context& c) const;

    private:
        StringLiteral                               _name;
        StringLiteral                               _description;
        std::function<Optional<Error> (const StringView&, const Context&)>    _callback;
    };


    class Command;
    using CommandDict = std::map<StringView, Command>;

    /**
     * Command for CLI
     */
    class Command {
    public:

        ~Command() = default;

        Command(const Command& rhs) = default;

        Command(Command&& rhs) = default;

        template<typename F>
        Command(StringView description, F&& f) :
            _description(std::move(description)),
            _callback(std::forward<F>(f)),
            _options()
        {}

        template<typename F>
        Command(StringView description,
                F&& f,
                const std::initializer_list<Option>& options) :
            _description(std::move(description)),
            _callback(std::forward<F>(f)),
            _options(options),
            _commands(),
            _arguments()
        {}

        template<typename F>
        Command(StringView description,
                const std::initializer_list<Argument>& arguments,
                F&& f) :
            _description(std::move(description)),
            _callback(std::forward<F>(f)),
            _options(),
            _commands(),
            _arguments(arguments)
        {}

        template<typename F>
        Command(StringView description,
                const std::initializer_list<Argument>& arguments,
                F&& f,
                const std::initializer_list<Option>& options) :
            _description(std::move(description)),
            _callback(std::forward<F>(f)),
            _options(options),
            _commands(),
            _arguments(arguments)
        {}

        Command& operator= (const Command& rhs) noexcept {
            Command(rhs).swap(*this);

            return *this;
        }

        Command& operator= (Command&& rhs) noexcept {
            return swap(rhs);
        }

        Command& swap(Command& rhs) noexcept {
            std::swap(_description, rhs._description);
            std::swap(_callback, rhs._callback);
            std::swap(_options, rhs._options);
            std::swap(_commands, rhs._commands);
            std::swap(_arguments, rhs._arguments);

            return (*this);
        }

        const StringView& description() const noexcept           { return _description; }
        Command& description(StringView description) noexcept {
            _description = description;
            return *this;
        }

        const Array<Option>& options() const noexcept               { return _options; }
        Command& options(std::initializer_list<Option> options) {
            _options = options;
            return *this;
        }

        const CommandDict&  commands() const noexcept  { return _commands; }
        Command& commands(std::initializer_list<CommandDict::value_type> commands) {
            _commands = commands;
            return *this;
        }

        const Array<Argument>& arguments() const noexcept           { return _arguments; }
        Command& arguments(std::initializer_list<Argument> arguments) {
            _arguments = arguments;
            return *this;
        }

        std::function<Result<void, Error>()> action() const {
            return _callback;
        }

        template<typename F>
        Command& action(F&& f) {
            _callback = std::forward<F>(f);
            return *this;
        }


    private:
        StringView                              _description;
        std::function<Result<void, Error>()>    _callback;

        /// Options / flags that the command accepts.
        Array<Option>   _options;

        /// Sub-commands
        CommandDict     _commands;

        /// Mandatory positional arguments
        Array<Argument> _arguments;
    };


public:

    //!< Default prefix for flags and options
    static const char DefaultPrefix;

    //!< Default value separator
    static const char DefaultValueSeparator;

public:

    ~Parser() = default;

    /// Default copy-constructor
    Parser(const Parser& rhs) = default;

    /// Default move-constructor
    Parser(Parser&& rhs) noexcept = default;

    /**
     * Construct default command line parser.
     *
     * @param appDescription Human readable application description to be used by 'help'-type commands.
     */
    Parser(StringView appDescription);

    /**
     * Construct a commandline parser with application description and a list of expected options.
     * @param appDescription Hyman readable application description to be used by 'help'-type commands.
     * @param options Initializer-list of command line options. @see CommandlineParser::options() for more info.
     */
    Parser(StringView appDescription,
                      const std::initializer_list<Option>& options);

    Parser& operator= (const Parser& rhs) noexcept {
        Parser(rhs).swap(*this);

        return *this;
    }

    Parser& operator= (Parser&& rhs) noexcept {
        return swap(rhs);
    }

    Parser& swap(Parser& rhs) noexcept {
        using std::swap;
        swap(_prefix, rhs._prefix);
        swap(_valueSeparator, rhs._valueSeparator);
        swap(_defaultAction, rhs._defaultAction);

        return (*this);
    }


    using ParseResult = std::function<Result<void, Error>()>;
    /**
     * Parse command line arguments and process all the flags.
     * @param argc Number of command line arguments including name of the program at argv[0]
     * @param argv An array of string that represent command line argument tokens.
     * @return Result of parsing: Either a pointer to the parser or an error.
     */
    Result<ParseResult, Error>
    parse(int argc, const char* argv[]) const;


    /**
     * Add an option to print application version.
     * @param appName Name of the application to print along with version.
     * @param appVersion Application version to be printed.
     * @return A parser option that when given by a user will result in a printing of the version info.
     */
    static Option printVersion(const StringView& appName, const Version& appVersion);


    /**
     * Add a command to print application's version.
     * @param appName Name of the application to print along with version.
     * @param appVersion Application version to be printed.
     * @return A parser command that when given by a user will result in a printing of the version info.
     */
    static CommandDict::value_type printVersionCmd(StringView appName, const Version& appVersion);


    /**
     * Add an option to print help summary.
     * @return A parser option that when given by a user will result in a printing of
     * short options summary.
     */
    static Option printHelp();

    /**
     * Add a command to print help summary.
     * @return A parser command that when given by a user will result in a printing of help summary.
     */
    static CommandDict::value_type printHelpCmd();


    /**
     * Get prefix used to identify flags and options.
     * @return prefix for flags and options.
     */
    char optionPrefix() const noexcept { return _prefix; }

    /**
     * Set prefix used to identify flags and options.
     * @param prefixChar A new value for the prefix character.
     * @return Reference to this for fluent interface.
     */
    Parser& optionPrefix(char prefixChar) noexcept {
        _prefix = prefixChar;
        return *this;
    }

    /**
     * Get prefix used to identify flags and options.
     * @return prefix for flags and options.
     */
    char valueSeparator() const noexcept { return _valueSeparator; }

    /**
     * Set prefix used to identify flags and options.
     * @param prefixChar A new value for the prefix character.
     * @return Reference to this for fluent interface.
     */
    Parser& valueSeparator(char value) noexcept {
        _valueSeparator = value;
        return *this;
    }


    /**
     * Get human readable description of the application, dispayed by help and version commands.
     * @return Human readable application description string.
     */
    const StringView& description() const noexcept { return _defaultAction.description(); }

    /**
     * Set human-readable application description, to be dispayed by 'help' and 'version' commands.
     * @param desc New value for human-readable application description string.
     * @return Reference to this for fluent interface.
     */
    Parser& description(StringView desc) noexcept {
        _defaultAction.description(desc);

        return *this;
    }

    const Array<Option>& options() const noexcept       { return _defaultAction.options(); }
    Parser& options(std::initializer_list<Option> options) {
        _defaultAction.options(options);

        return *this;
    }

    const CommandDict& commands() const noexcept        { return _defaultAction.commands(); }
    Parser& commands(std::initializer_list<CommandDict::value_type> commands) {
        _defaultAction.commands(commands);

        return *this;
    }

    const Array<Argument>& arguments() const noexcept       { return _defaultAction.arguments(); }
    Parser& arguments(std::initializer_list<Argument> arguments) {
        _defaultAction.arguments(arguments);

        return *this;
    }

    Command& defaultAction() noexcept { return _defaultAction; }
    const Command& defaultAction() const noexcept { return _defaultAction; }

    template<typename F>
    const Command& defaultAction(F&& f) {
        _defaultAction.action(std::forward<F>(f));
        return _defaultAction;
    }

private:

    /// Option prefix
    char            _prefix;

    /// Value separator
    char            _valueSeparator;

    /// Default action to be produced when no other commands specified.
    Command         _defaultAction;
};



inline void swap(Parser::Option& lhs, Parser::Option& rhs) noexcept {
    lhs.swap(rhs);
}

inline void swap(Parser::Command& lhs, Parser::Command& rhs) noexcept {
    lhs.swap(rhs);
}

inline void swap(Parser& lhs, Parser& rhs) noexcept {
    lhs.swap(rhs);
}

}  // End of namespace cli
}  // End of namespace Solace
#endif  // SOLACE_CLI_PARSER_HPP
