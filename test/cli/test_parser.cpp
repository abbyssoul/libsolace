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
 * libSolace Unit Test Suit
 * @file: test/cli/test_parser.cpp
 * @author: soultaker
 *
 * Created on: 20/06/2016
*******************************************************************************/
#include <solace/cli/parser.hpp>  // Class being tested

#include <gtest/gtest.h>

#include <solace/parseUtils.hpp>

using namespace Solace;
using namespace Solace::cli;

class TestCommandlineParser: public ::testing::Test {

public:

    template<typename T>
    void testIntParsing(const char* strArg, T expectedValue, bool expectedToPass = true) {
        T parsedValue { 0 };

        const char* argv[] = {"prog", "-x", strArg, nullptr};
        const auto result = Parser("Something awesome", {
                              {{"x", "xxx"}, "Something", &parsedValue}
                          })
                .parse(countArgc(argv), argv);

        EXPECT_EQ(static_cast<T>(expectedValue), parsedValue);
        EXPECT_EQ(expectedToPass, result ? true : false);
    }

    template<typename T>
    void testIntOverflow(const char* strArg) {
        testIntParsing<T>(strArg, 0, false);
    }

    static int countArgc(const char* argv[]) {
        int count = 0;
        while (argv[count] != nullptr)
            ++count;

        return count;
    }
};

TEST_F(TestCommandlineParser, parseNullString) {
    EXPECT_TRUE(Parser("Something awesome")
                    .parse(0, nullptr)
                    .isOk());
}

TEST_F(TestCommandlineParser, parseEmptyString) {
    const char* argv[] = {""};

    EXPECT_TRUE(Parser("Something awesome")
                    .parse(0, argv)
                    .isOk());

    EXPECT_TRUE(Parser("Something awesome")
                    .parse(1, argv)
                    .isOk());
}

TEST_F(TestCommandlineParser, parseOneArgumentString) {
    const char* argv[] = {"blarg!"};

    EXPECT_TRUE(Parser("Something awesome", {})
                    .parse(1, argv)
                    .isOk());
}

TEST_F(TestCommandlineParser, parseNegativeNumberOfArgument) {
    const char* argv[] = {"blarg!"};

    EXPECT_TRUE(Parser("Something awesome")
                    .parse(-31, argv)
                    .isError());
}

TEST_F(TestCommandlineParser, parseInt8) {
    testIntParsing<int8>("120", 120);
}

TEST_F(TestCommandlineParser, parseInt8_Overflow) {
    testIntOverflow<int8>("32042");
}

TEST_F(TestCommandlineParser, parseUInt8) {
    testIntParsing<uint8>("240", 240);
}

TEST_F(TestCommandlineParser, parseUInt8_Overflow) {
    testIntOverflow<uint8>("429883");
}

TEST_F(TestCommandlineParser, parseUInt8_NegativeArgument) {
    testIntParsing<uint8>("-32", 0, false);
}

TEST_F(TestCommandlineParser, parseUInt8_NegativeOverflow) {
    testIntParsing<uint8>("-739834887", 0, false);
}

TEST_F(TestCommandlineParser, parseInt16) {
    testIntParsing<int16>("321", 321);
}

TEST_F(TestCommandlineParser, parseInt16_Overflow) {
    testIntOverflow<int16>("68535");
}

TEST_F(TestCommandlineParser, parseUInt16) {
    testIntParsing<uint16>("9883", 9883);
}

TEST_F(TestCommandlineParser, parseUInt16_Overflow) {
    testIntOverflow<uint16>("429883");
}

TEST_F(TestCommandlineParser, parseUInt16_NegativeArgument) {
    testIntParsing<uint16>("-73", 0, false);
}

TEST_F(TestCommandlineParser, parseUInt16_NegativeOverflow) {
    testIntParsing<uint16>("-739834887", 0, false);
}

TEST_F(TestCommandlineParser, parseInt32) {
    testIntParsing<int32>("717321", 717321);
}

TEST_F(TestCommandlineParser, parseInt32_Overflow) {
    testIntOverflow<int32>("9898847598475");
}

TEST_F(TestCommandlineParser, parseUInt32) {
    testIntParsing<uint32>("19587446", 19587446);
}

TEST_F(TestCommandlineParser, parseUInt32_Overflow) {
    testIntOverflow<uint32>("4298833432");
}

TEST_F(TestCommandlineParser, parseUInt32_NegativeArgument) {
    testIntParsing<uint32>("-19587446", 0, false);
}

TEST_F(TestCommandlineParser, parseUInt32_NegativeOverflow) {
    testIntParsing<uint32>("-9898847598475", 0, false);
}

TEST_F(TestCommandlineParser, parseInt64) {
    testIntParsing<int64>("717321", 717321);
}

TEST_F(TestCommandlineParser, parseInt64_Overflow) {
    testIntOverflow<int64>("9898847598475978947899839987438957");
    testIntOverflow<int64>("922337203685477580742111");
}

TEST_F(TestCommandlineParser, parseUInt64) {
    testIntParsing<uint64>("19587446", 19587446);
}

TEST_F(TestCommandlineParser, parseUInt64_Overflow) {
    testIntOverflow<uint64>("9898847598475978947899839987438957");
    testIntOverflow<uint64>("92233720368547758072");
}

TEST_F(TestCommandlineParser, parseUInt64_NegativeArgument) {
    testIntParsing<uint64>("-19587446", 0, false);
}

TEST_F(TestCommandlineParser, parseUInt64_NegativeOverflow) {
    testIntParsing<uint64>("-922337203685477580712", 0, false);
}

TEST_F(TestCommandlineParser, testBoolWithNoArgument) {
    bool parsedSuccessully = false;

    bool xValue = false;
    int vValue = 0;

    const char* argv[] = {"prog", "--xxx", "-V", "321", nullptr};
    const char* appDesc = "Something awesome";
    Parser(appDesc, {
                            {{"x", "xxx"}, "Something", &xValue},
                            {{"V", "vvv"}, "Something else", &vValue}
                        })
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error&& e) {
                    parsedSuccessully = false;

                    FAIL() << (e.toString().data());
                });

    EXPECT_TRUE(parsedSuccessully);
    EXPECT_EQ(true, xValue);
    EXPECT_EQ(321, vValue);
}

TEST_F(TestCommandlineParser, testUnrecognizedArgument) {
    bool parsedSuccessully = false;
    int xValue = 0;

    const char* argv[] = {"prog", "--xxx", "756", "--unknown", nullptr};
    const char* appDesc = "Something awesome";
    Parser(appDesc, {
                            {{"x", "xxx"}, "Something", &xValue}
                        })
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error&&){parsedSuccessully = false;});


    EXPECT_TRUE(!parsedSuccessully);
    EXPECT_EQ(756, xValue);
}

TEST_F(TestCommandlineParser, testOptionalValueAndUnrecognizedArgument) {
    bool parsedSuccessully = false;
    bool xValue = false;

    const char* argv[] = {"prog", "-v", "--unknown", "blah!", nullptr};
    const char* appDesc = "Something awesome";
    Parser(appDesc, {
                            {{"v", "vvv"}, "Something", &xValue}
                        })
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error&&) {parsedSuccessully = false;});


    EXPECT_TRUE(!parsedSuccessully);
    EXPECT_EQ(true, xValue);
}

TEST_F(TestCommandlineParser, testNoShortValue) {
    bool parsedSuccessully = false;
    int xValue = 0;

    const char* argv[] = {"prog", "-x", nullptr};
    const char* appDesc = "Something awesome";
    Parser(appDesc, {
                            {{"x", "xxx"}, "Something", &xValue}
                        })
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error&&) {parsedSuccessully = false;});


    EXPECT_TRUE(!parsedSuccessully);
    EXPECT_EQ(0, xValue);
}

TEST_F(TestCommandlineParser, testNoLongValue) {

    bool parsedSuccessully = false;
    int xValue = 0;

    const char* argv[] = {"prog", "--xxx", nullptr};
    const char* appDesc = "Something awesome";
    Parser(appDesc, {
                            {{"x", "xxx"}, "Something", &xValue}
                        })
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error&&){parsedSuccessully = false;});


    EXPECT_TRUE(!parsedSuccessully);
    EXPECT_EQ(0, xValue);
}

TEST_F(TestCommandlineParser, testInvalidValueType) {

    bool parsedSuccessully = false;
    int xValue = 0;

    const char* argv[] = {"prog", "--xxx", "BHAL!", nullptr};
    const char* appDesc = "Something awesome";
    Parser(appDesc, {
                            {{"x", "xxx"}, "Something", &xValue}
                        })
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error&&){parsedSuccessully = false;});


    EXPECT_TRUE(!parsedSuccessully);
    EXPECT_EQ(0, xValue);
}

TEST_F(TestCommandlineParser, testEmptyName) {
    bool parsedSuccessully = false;
    int xValue = 0;

    const char* argv[] = {"prog", "", "-xy", "32", nullptr};
    const char* appDesc = "Something awesome";
    Parser(appDesc, {{{"x", "xxx"}, "Something", &xValue}})
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error&&){parsedSuccessully = false;});


    EXPECT_EQ(0, xValue);
    EXPECT_TRUE(!parsedSuccessully);
}

TEST_F(TestCommandlineParser, testSinglePrefix) {
    bool parsedSuccessully = false;
    int xValue = 0;

    const char* argv[] = {"prog", "-", "32", nullptr};
    const char* appDesc = "Something awesome";
    Parser(appDesc, {{{"x", "xxx"}, "Something", &xValue}})
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error&&){parsedSuccessully = false;});


    EXPECT_EQ(0, xValue);
    EXPECT_TRUE(!parsedSuccessully);
}

TEST_F(TestCommandlineParser, testDoublePrefix) {
    bool parsedSuccessully = false;
    int xValue = 0;

    const char* argv[] = {"prog", "--", "BHAL!", nullptr};
    const char* appDesc = "Something awesome";
    Parser(appDesc, {{{"x", "xxx"}, "Something", &xValue}})
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error&&){parsedSuccessully = false;});


    EXPECT_EQ(0, xValue);
    EXPECT_TRUE(!parsedSuccessully);
}

TEST_F(TestCommandlineParser, testCustomHandlerLong) {
    bool parsedSuccessully = false;
    bool customCalled = false;
    int xValue = 0;
    StringView zValue;
    const StringView zExpValue("somethin");

    const char* argv[] = {"prog", "--xxx", "756", "--zva", "somethin", nullptr};
    const char* appDesc = "Something awesome";
    Parser(appDesc, {
                    {{"x", "xxx"}, "Something", &xValue},
                    {{"z", "zva"}, "Custom arg", Parser::OptionArgument::Required,
                    [&customCalled, &zValue](const Optional<StringView>& value, const Parser::Context&) {
                        customCalled = true;
                        zValue = value.get();

                        return none;
                    }}
                })
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) { parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error&&) { parsedSuccessully = false; });


    EXPECT_TRUE(parsedSuccessully);
    EXPECT_TRUE(customCalled);
    EXPECT_EQ(756, xValue);
    EXPECT_EQ(zExpValue, zValue);
}

TEST_F(TestCommandlineParser, testCustomHandlerShort) {
    bool parsedSuccessully = false;
    bool customCalled = false;
    int xValue = 0;
    StringView zValue;
    const StringView zExpValue("somethin2");

    const char* argv[] = {"prog", "--xxx", "756", "-z", "somethin2", nullptr};
    const char* appDesc = "Something awesome";
    Parser(appDesc, {
                    {{"x", "xxx"}, "Something", &xValue},
                    {{"z", "zve"}, "Custom arg", Parser::OptionArgument::Required,
                    [&customCalled, &zValue](const Optional<StringView>& value, const Parser::Context&) {
                        customCalled = true;
                        zValue = value.get();

                        return none;
                    }}
                })
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error){parsedSuccessully = false;});


    EXPECT_TRUE(parsedSuccessully);
    EXPECT_TRUE(customCalled);
    EXPECT_EQ(756, xValue);
    EXPECT_EQ(zExpValue, zValue);
}

TEST_F(TestCommandlineParser, testCustomNoValue) {
    bool customCalled = false;
    int xValue = 0;
    StringView zValue;

    const char* argv[] = {"prog", "--xxx", "756", "-z", nullptr};
    const char* appDesc = "Something awesome";
    const auto result = Parser(appDesc, {
                    {{"x", "xxx"}, "Something", &xValue},
                    {{"z", "zve"}, "Custom arg", Parser::OptionArgument::Required,
                    [&customCalled, &zValue](const Optional<StringView>& value, const Parser::Context&) {
                        customCalled = true;
                        zValue = value.get();

                        return none;
                    }}
                })
            .parse(countArgc(argv), argv);


    EXPECT_TRUE(!result);
    EXPECT_TRUE(!customCalled);
    EXPECT_EQ(756, xValue);
}

TEST_F(TestCommandlineParser, testCustomNoValueExpected) {
    bool parsedSuccessully = false;
    bool customCalled = false;
    int xValue = 0;

    const char* argv[] = {"prog", "--xxx", "756", "-z", nullptr};
    const char* appDesc = "Something awesome";
    Parser(appDesc, {
                            {{"x", "xxx"}, "Something", &xValue},
                            {{"z", "zve"}, "Custom arg", Parser::OptionArgument::NotRequired,
                            [&customCalled](const Optional<StringView>&, const Parser::Context&) {
                                customCalled = true;

                                return none;
                            } }
                        })
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) { parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


    EXPECT_TRUE(parsedSuccessully);
    EXPECT_TRUE(customCalled);
    EXPECT_EQ(756, xValue);
}

TEST_F(TestCommandlineParser, testCustomNoValueExpectedButGiven) {
    bool parsedSuccessully = false;
    bool customCalled = false;
    int xValue = 0;
    bool zVal;
    StringView argStr;

    const char* argv[] = {"prog", "--xxx", "756", "--zve", "Val", nullptr};
    Parser("Something awesome", {
                            {{"x", "xxx"}, "Something", &xValue},
                            {{"z", "zve"}, "Custom arg", Parser::OptionArgument::NotRequired,
                            [&customCalled, &zVal](const Optional<StringView>& val, const Parser::Context&) {
                                customCalled = true;
                                zVal = val.isSome();

                                return none;
                            } }
                        })
            .arguments({{"arg", "arg sink", &argStr}})
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) { parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


    EXPECT_TRUE(parsedSuccessully);
    EXPECT_TRUE(customCalled);
    EXPECT_EQ(756, xValue);
    EXPECT_EQ(false, zVal);
    EXPECT_EQ(StringView("Val"), argStr);
}

TEST_F(TestCommandlineParser, testInlineValues) {
    bool parsedSuccessully = false;

    bool xValue = false;
    int vValue = 0;
    StringView sValue;

    const char* argv[] = {"prog", "--bool=1", "-V=321", "--string=blah!", nullptr};
    Parser("Something awesome", {
                            {{"b", "bool"}, "BooleanValue", &xValue},
                            {{"V", "vvv"}, "IntValue", &vValue},
                            {{"s", "string"}, "String value", &sValue}
                        })
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error&& e) {
                    parsedSuccessully = false;

                    FAIL() << (e.toString().data());
                });

    EXPECT_TRUE(parsedSuccessully);
    EXPECT_EQ(true, xValue);
    EXPECT_EQ(321, vValue);
    EXPECT_EQ(sValue, "blah!");
}

TEST_F(TestCommandlineParser, testInlineValuesTypeMismatch) {
    int vValue = 0;
    StringView sValue;

    const char* argv[] = {"prog", "--intValue=Hello", nullptr};
    const auto result = Parser("Something awesome", {
                            {{"intValue"}, "Int Value", &vValue},
                            {{"s", "string"}, "String value", &sValue}
                        })
            .parse(countArgc(argv), argv);

    EXPECT_TRUE(result.isError());
}

TEST_F(TestCommandlineParser, testRepeatingOptions_Int) {
    int vValue = 0;
    bool unusedValue;
    bool parsedSuccessully = false;

    const char* argv[] = {"prog",
                            "--intValue=32",
                            "-i", "17",
                            "-v",
                            "--intValue", "918",
                            nullptr};

    Parser("Something awesome", {
                            {{"i", "intValue"}, "Int Value", &vValue},
                            {{"v"}, "Useless value", &unusedValue}
                        })
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error&& e) {
                    parsedSuccessully = false;

                    FAIL() << (e.toString().data());
                });

    EXPECT_TRUE(parsedSuccessully);
    EXPECT_EQ(918, vValue);
}

TEST_F(TestCommandlineParser, testRepeatingOptionsCustomHandler) {
    int vValue = 0;
    bool unusedValue;
    bool parsedSuccessully = false;

    const char* argv[] = {"prog",
                            "--intValue=32",
                            "-i", "17",
                            "-v",
                            "--intValue", "918",
                            nullptr};

    Parser("Something awesome")
            .options({
                        {{"i", "intValue"}, "Value",
                        Parser::OptionArgument::Required,
                        [&vValue](const Optional<StringView>& value, const Parser::Context&) -> Optional<Error>{
                            auto res = tryParse<int>(value.get());
                            if (res) {
                                vValue += res.unwrap();
                                return none;
                            } else {
                                return Optional<Error>(res.moveError());
                            }
                        }},
                        {{"v"}, "Useless value", &unusedValue}
                    })
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error&& e) {
                    parsedSuccessully = false;

                    FAIL() << (e.toString().data());
                });

    EXPECT_TRUE(parsedSuccessully);
    EXPECT_EQ(918 + 32 + 17, vValue);
}

TEST_F(TestCommandlineParser, testRepeatingOptionsWithDifferentType) {
    int vValue = 0;
    bool unusedValue;

    const char* argv[] = {"prog",
                            "--intValue=32",
                            "-i", "Info",
                            "-v",
                            "--intValue", "918",
                            nullptr};

    const auto result = Parser("Something awesome", {
                            {{"i", "intValue"}, "Int Value", &vValue},
                            {{"v"}, "Useless value", &unusedValue}
                        })
            .parse(countArgc(argv), argv);

    EXPECT_TRUE(result.isError());
}

TEST_F(TestCommandlineParser, testMandatoryArgument) {
    bool parsedSuccessully = false;
    int xValue = 0;
    int mandatoryArg = 0;

    const char* argv[] = {"prog", "-x", "756", "98765", nullptr};
    Parser("Something awesome", {
                            {{"x", "xxx"}, "Something", &xValue}
            })
            .arguments({
                {"manarg", "Mandatory argument", &mandatoryArg}
            })
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) { parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error) { parsedSuccessully = false;});


    EXPECT_TRUE(parsedSuccessully);
    EXPECT_EQ(756, xValue);
    EXPECT_EQ(98765, mandatoryArg);
}

TEST_F(TestCommandlineParser, testMandatoryArgumentOnly) {
    bool parsedSuccessully = false;
    StringView mandatoryArg;

    const char* argv[] = {"prog", "awesome-value", nullptr};
    Parser("Something awesome")
            .arguments({
                {"manarg", "Mandatory argument", &mandatoryArg}
            })
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) { parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


    EXPECT_TRUE(parsedSuccessully);
    EXPECT_EQ(StringView("awesome-value"), mandatoryArg);
}

TEST_F(TestCommandlineParser, testMandatoryArgumentWithoutGivenFlags) {
    bool parsedSuccessully = false;
    int xValue = 0;
    StringView mandatoryArg;

    const char* argv[] = {"prog", "awesome-value", nullptr};
    Parser("Something awesome", {
                            {{"x", "xxx"}, "Something", &xValue}
                        })
            .arguments({
                            {"manarg", "Mandatory argument", &mandatoryArg}
                })
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) { parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


    EXPECT_TRUE(parsedSuccessully);
    EXPECT_EQ(0, xValue);
    EXPECT_EQ(StringView("awesome-value"), mandatoryArg);
}

TEST_F(TestCommandlineParser, testMandatoryArgumentMissing) {
    bool parsedSuccessully = false;
    int xValue = 0;
    int mandatoryArg = 0;

    const char* argv[] = {"prog", "-x", "756", nullptr};
    Parser("Something awesome", {
                {{"x", "xxx"}, "Something", &xValue}
            })
            .arguments({
                {"manarg", "Mandatory argument", &mandatoryArg}
            })
            .parse(countArgc(argv), argv)
            .then([&parsedSuccessully](Parser::ParseResult&&) { parsedSuccessully = true; })
            .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });

    EXPECT_TRUE(!parsedSuccessully);
    EXPECT_EQ(0, mandatoryArg);
    EXPECT_EQ(756, xValue);
}

TEST_F(TestCommandlineParser, testMandatoryArgumentNotEnough) {
    StringView mandatoryArgStr;
    int mandatoryArgInt = 0;
    int mandatoryArgInt2 = 0;

    const char* argv[] = {"prog", "do", "321", nullptr};
    EXPECT_TRUE(Parser("Something awesome")
            .arguments({
                {"manarg1", "Mandatory argument", &mandatoryArgStr},
                {"manarg2", "Mandatory argument", &mandatoryArgInt},
                {"manarg3", "Mandatory argument", &mandatoryArgInt2}
            })
            .parse(countArgc(argv), argv)
            .isError());
}

TEST_F(TestCommandlineParser, testMandatoryArgumentTooMany) {
    StringView mandatoryArgStr;
    int mandatoryArgInt = 0;

    const char* argv[] = {"prog", "some", "756", "other", nullptr};
    EXPECT_TRUE(Parser("Something awesome")
            .arguments({
                {"manarg1", "Mandatory argument", &mandatoryArgStr},
                {"manarg2", "Mandatory argument", &mandatoryArgInt}
            })
            .parse(countArgc(argv), argv)
            .isError());
}

TEST_F(TestCommandlineParser, testTrailingArgumentsWithRegular) {
    int nbTimesInvoked = 0;
    StringView mandatoryArgStr;
    StringView lastTrailingArg;

    const char* argv[] = {"prog", "some", "756", "other", nullptr};
    const auto result = Parser("Something awesome")
            .arguments({
                {"manarg1", "Mandatory argument", &mandatoryArgStr},
                {"*", "Input",
                [&nbTimesInvoked, &lastTrailingArg](StringView v, const Parser::Context&) -> Optional<Error>
                {
                    nbTimesInvoked += 1;
                    lastTrailingArg = v;

                    return none;
                }}
            })
            .parse(countArgc(argv), argv);

    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(StringView("some"), mandatoryArgStr);
    EXPECT_EQ(2, nbTimesInvoked);
    EXPECT_EQ(StringView("other"), lastTrailingArg);
}

TEST_F(TestCommandlineParser, testTrailingArguments) {
    int nbTimesInvoked = 0;

    const char* argv[] = {"prog", "some", "756", "other", nullptr};
    const auto result = Parser("Something awesome")
            .arguments({
                {"*", "Input",
                 [&nbTimesInvoked](StringView, const Parser::Context&) -> Optional<Error> {
                    nbTimesInvoked += 1;
                    return none;
                }}
            })
            .parse(countArgc(argv), argv);

    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(3, nbTimesInvoked);
}

TEST_F(TestCommandlineParser, testTrailingNoArguments) {
    int nbTimesInvoked = 0;

    const char* argv[] = {"prog", nullptr};
    const auto result = Parser("Something awesome")
            .arguments({
                {"*", "Input", [&nbTimesInvoked](StringView, const Parser::Context&) -> Optional<Error>
                {
                    nbTimesInvoked += 1;
                    return none;
                }}
            })
            .parse(countArgc(argv), argv);

    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(0, nbTimesInvoked);
}

TEST_F(TestCommandlineParser, testTrailingNoArgumentsLeft) {
    int nbTimesInvoked = 0;

    StringView mandatoryArgStr1;
    StringView mandatoryArgStr2;

    const char* argv[] = {"prog", "man1", "man2", nullptr};
    const auto result = Parser("Something awesome")
            .arguments({
                {"manarg1", "Mandatory argument", &mandatoryArgStr1},
                {"manarg2", "Mandatory argument", &mandatoryArgStr2},
                {"*", "Input", [&nbTimesInvoked](StringView, const Parser::Context&) -> Optional<Error>
                {
                    nbTimesInvoked += 1;
                    return none;
                }}
            })
            .parse(countArgc(argv), argv);

    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(StringView("man1"), mandatoryArgStr1);
    EXPECT_EQ(StringView("man2"), mandatoryArgStr2);
    EXPECT_EQ(0, nbTimesInvoked);
}

TEST_F(TestCommandlineParser, testTrailingArgumentsWithOptions) {
    int nbTimesInvoked = 0;
    int optValue = 0;
    StringView lastTrailingArg;

    const char* argv[] = {"prog", "--opt", "756", "maybe_not", nullptr};
    const auto result = Parser("Something awesome")
            .options({
                {{"opt"}, "Option", &optValue}
            })
            .arguments({
                {"*", "Input",
                [&nbTimesInvoked, &lastTrailingArg](StringView v, const Parser::Context&) -> Optional<Error>
                {
                    nbTimesInvoked += 1;
                    lastTrailingArg = v;

                    return none;
                }}
            })
            .parse(countArgc(argv), argv);

    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(756, optValue);
    EXPECT_EQ(1, nbTimesInvoked);
    EXPECT_EQ(StringView("maybe_not"), lastTrailingArg);
}

TEST_F(TestCommandlineParser, testCommandGivenButNotExpected) {
    bool commandExecuted = false;
    bool givenOpt = false;

    const char* argv[] = {"prog", "command", nullptr};
    const auto result = Parser("Something awesome", {
                            {{"b", "bsome"}, "Some option", &givenOpt}
                        })
            .parse(countArgc(argv), argv);

    EXPECT_TRUE(result.isError());
    EXPECT_TRUE(!commandExecuted);
}

TEST_F(TestCommandlineParser, testMandatoryCommandNotGiven) {
    bool commandExecuted = false;

    const char* argv[] = {"prog", nullptr};
    const auto result = Parser("Something awesome")
            .commands({
                {"doThings", {"Mandatory command",
                            [&commandExecuted]() -> Result<void, Error> {
                                commandExecuted = true;

                                return Ok();
                            }
                }}
            })
            .parse(countArgc(argv), argv);

    EXPECT_TRUE(!commandExecuted);
    EXPECT_TRUE(result.isError());
}

TEST_F(TestCommandlineParser, testMandatoryCommandWithNoArgumentsSuccess) {
    bool commandExecuted = false;

    const char* argv[] = {"prog", "doIt", nullptr};
    const auto result = Parser("Something awesome")
            .commands({
                            {"doIt", {"Pass the test",
                            [&commandExecuted]() -> Result<void, Error> {
                                commandExecuted = true;
                                return Ok();
                            }}}
                        })
            .parse(countArgc(argv), argv);

    if (!result) {
        FAIL() << (result.getError().toString().data());
    } else {
        EXPECT_TRUE(result.unwrap()().isOk());
    }

    EXPECT_TRUE(commandExecuted);
}

TEST_F(TestCommandlineParser, testInvalidCommand) {
    bool commandExecuted = false;

    const char* argv[] = {"prog", "somethingElse", nullptr};
    EXPECT_TRUE(Parser("Something awesome")
            .commands({
                            {"doIt", {"Pass the test",
                            [&commandExecuted]() -> Result<void, Error> {
                                commandExecuted = true;
                                return Ok();
                            }}}
                        })
            .parse(countArgc(argv), argv)
            .isError());

    EXPECT_TRUE(!commandExecuted);
}

TEST_F(TestCommandlineParser, testInvalidArgumentsForCommand) {
    bool commandExecuted = false;


    struct CmdCntx {
        bool all;
    } cmdCntx;

    const char* argv[] = {"prog", "doIt", "b", "blah!", nullptr};
    EXPECT_TRUE(Parser("Something awesome")
            .commands({
                            {"doIt", {"Pass the test",
                            [&commandExecuted]() -> Result<void, Error> {
                                commandExecuted = true;
                                return Ok();
                            }, {
                                {{"a", "all"}, "Do something everywhere", &cmdCntx.all}
                            }}}
                        })
            .parse(countArgc(argv), argv)
            .isError());

    EXPECT_TRUE(!commandExecuted);
}

TEST_F(TestCommandlineParser, testUnexpectedArgumentsForCommand) {
    bool commandExecuted = false;

    struct CmdCntx {
        bool all;
    } cmdCntx;

    const char* argv[] = {"prog", "doIt", "-a", "-b", "blah!", nullptr};
    EXPECT_TRUE(Parser("Something awesome")
            .commands({
                            {"doIt", {"Pass the test",
                            [&commandExecuted]() -> Result<void, Error> {
                                commandExecuted = true;

                                return Ok();
                            }, {
                                {{"a", "all"}, "Do something everywhere", &cmdCntx.all}
                            }}}
                        })
            .parse(countArgc(argv), argv)
            .isError());

    EXPECT_TRUE(!commandExecuted);
}

TEST_F(TestCommandlineParser, multipleCommandSelection) {
    bool commandExecuted[] = {false, false};

    const char* argv[] = {"prog", "comm-1", nullptr};
    auto parseResult = Parser("Something awesome")
            .commands({
                {"comm-1", {"Run 1st command",
                [&]() -> Result<void, Error> {
                    commandExecuted[0] = true;
                    return Ok();
                }}},

                {"comm-2", {"Run 2nd command",
                [&]() -> Result<void, Error> {
                    commandExecuted[1] = true;
                    return Ok();
                }}}
            })
            .parse(countArgc(argv), argv);

    if (!parseResult) {
        FAIL() << (parseResult.getError().toString().data());
    } else {
        EXPECT_TRUE(parseResult.unwrap()().isOk());
    }

    EXPECT_TRUE(commandExecuted[0]);
    EXPECT_TRUE(!commandExecuted[1]);
}

TEST_F(TestCommandlineParser, multipleCommandWithSimilarOptions) {
    bool commandExecuted[] = {false, false, false};

    struct Cmd2Options {
        int commonFlag;
        int value;
        float32 otherValue;
    } cmd2Options;

    struct Cmd1Options {
        int commonFlag;
        StringView value;
    } cmd1Options;


    const char* argv[] = {"prog", "comm-2", "--commonOption", "321", nullptr};
    const auto result = Parser("Something awesome")
            .commands({
                            {"comm-1", {"Run 1st command",
                            [&]() -> Result<void, Error> {
                                commandExecuted[0] = true;
                                return Ok();
                            }, {
                                {{"c", "commonOption"}, "Common option", &cmd1Options.commonFlag},
                                {{"o", "uniqueOption"}, "Some unique option", &cmd1Options.value},
                            }}},

                            {"comm-2", {"Run 2nd command",
                            [&]() -> Result<void, Error> {
                                commandExecuted[1] = true;
                                return Ok();
                            }, {
                                {{"c", "commonOption"}, "Common option", &cmd2Options.commonFlag},
                                {{"o", "uniqueOption"}, "Some unique option", &cmd2Options.value},
                                {{"o", "uniqueOption"}, "Some unique option", &cmd2Options.otherValue}
                            }}},

                            {"comm-3", {"Run 3rd command",
                            [&]() -> Result<void, Error> {
                                commandExecuted[2] = true;

                                return Ok();
                            }}}
                        })
            .parse(countArgc(argv), argv);

    if (!result) {
        FAIL() << (result.getError().toString().data());
    } else {
        EXPECT_TRUE(result.unwrap()().isOk());
    }

    EXPECT_TRUE(!commandExecuted[0]);
    EXPECT_TRUE(commandExecuted[1]);
    EXPECT_EQ(321, cmd2Options.commonFlag);
    EXPECT_TRUE(!commandExecuted[2]);
}

TEST_F(TestCommandlineParser, commandExecutionFails) {
    bool commandExecuted[] = {false, false};

    const char* argv[] = {"prog", "comm-f", nullptr};
    const auto result = Parser("Something awesome")
            .commands({
                        {"comm-s", {"Run 1st command",
                            [&]() -> Result<void, Error> {
                                commandExecuted[0] = true;
                                return Ok();
                            }}},

                        {"comm-f", {"Run 2nd command",
                            [&]() -> Result<void, Error> {
                                commandExecuted[1] = true;

                                return Err(Error("As designed"));
                            }}}
                        })
            .parse(countArgc(argv), argv);

    if (!result) {
        FAIL() << (result.getError().toString().data());
    } else {
        EXPECT_TRUE(result.unwrap()().isError());
    }

    EXPECT_TRUE(!commandExecuted[0]);
    EXPECT_TRUE(commandExecuted[1]);
}

TEST_F(TestCommandlineParser, multipleCommandWithOptionsAndArguments) {
    bool commandExecuted[] = {false, false, false};


    bool verbose = false;
    int globalInt = 0;

    struct Cmd2Options {
        int commonFlag;
        int value;
        float32 otherValue;

        StringView arg1;
        StringView arg2;
    } cmd2Options;

    struct Cmd1Options {
        int commonFlag;
        StringView value;
    } cmd1Options;


    const char* argv[] = {"prog", "-v", "--intValue", "42", "comm-2", "-o", "11", "ArgValue1", "arg2", nullptr};
    const auto result = Parser("Something awesome")
            .options({
                            {{"v", "verbose"}, "Verbose output", &verbose},
                            {{"i", "intValue"}, "Global int", &globalInt}
                        })
            .commands({
                            {"comm-1", {"Run 1st command",
                            [&]() -> Result<void, Error> {
                                commandExecuted[0] = true;
                                return Ok();
                            }, {
                                {{"c", "commonOption"}, "Common option", &cmd1Options.commonFlag},
                                {{"o", "uniqueOption"}, "Some unique option", &cmd1Options.value},
                            }}},

                            {"comm-2", {"Run 2nd command", {
                                {"arg1", "Arg value1", &cmd2Options.arg1},
                                {"arg2", "Arg value2", &cmd2Options.arg2}
                            },
                            [&]() -> Result<void, Error> {
                                commandExecuted[1] = true;
                                return Ok();
                            }, {
                                {{"c", "commonOption"}, "Common option", &cmd2Options.commonFlag},
                                {{"o", "uniqueOption"}, "Some unique option", &cmd2Options.value}
                            }
                            }},

                            {"comm-3", {"Run 3rd command",
                            [&]() -> Result<void, Error> {
                                commandExecuted[2] = true;

                                return Ok();
                            }}}
                        })
            .parse(countArgc(argv), argv);

    if (!result) {
        FAIL() << (result.getError().toString().data());
    } else {
        EXPECT_TRUE(result.unwrap()().isOk());
    }

    EXPECT_TRUE(!commandExecuted[0]);
    EXPECT_TRUE(commandExecuted[1]);
    EXPECT_TRUE(!commandExecuted[2]);


    EXPECT_EQ(true, verbose);
    EXPECT_EQ(42, globalInt);

    EXPECT_EQ(11, cmd2Options.value);
    EXPECT_EQ(StringView("ArgValue1"), cmd2Options.arg1);
    EXPECT_EQ(StringView("arg2"), cmd2Options.arg2);
}
