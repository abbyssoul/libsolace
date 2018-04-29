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

#include <cppunit/extensions/HelperMacros.h>

#include <solace/parseUtils.hpp>


using namespace Solace;
using namespace Solace::cli;


class TestCommandlineParser: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestCommandlineParser);
        CPPUNIT_TEST(parseNullString);
        CPPUNIT_TEST(parseEmptyString);
        CPPUNIT_TEST(parseOneArgumentString);
        CPPUNIT_TEST(parseNegativeNumberOfArgument);

        CPPUNIT_TEST(parseInt8);
        CPPUNIT_TEST(parseInt8_Overflow);
        CPPUNIT_TEST(parseUInt8);
        CPPUNIT_TEST(parseUInt8_Overflow);
        CPPUNIT_TEST(parseUInt8_NegativeArgument);
        CPPUNIT_TEST(parseUInt8_NegativeOverflow);

        CPPUNIT_TEST(parseInt16);
        CPPUNIT_TEST(parseInt16_Overflow);
        CPPUNIT_TEST(parseUInt16);
        CPPUNIT_TEST(parseUInt16_Overflow);
        CPPUNIT_TEST(parseUInt16_NegativeArgument);
        CPPUNIT_TEST(parseUInt16_NegativeOverflow);

        CPPUNIT_TEST(parseInt32);
        CPPUNIT_TEST(parseInt32_Overflow);
        CPPUNIT_TEST(parseUInt32);
        CPPUNIT_TEST(parseUInt32_Overflow);
        CPPUNIT_TEST(parseUInt32_NegativeArgument);
        CPPUNIT_TEST(parseUInt32_NegativeOverflow);

        CPPUNIT_TEST(parseInt64);
        CPPUNIT_TEST(parseInt64_Overflow);
        CPPUNIT_TEST(parseUInt64);
        CPPUNIT_TEST(parseUInt64_Overflow);
        CPPUNIT_TEST(parseUInt64_NegativeArgument);
        CPPUNIT_TEST(parseUInt64_NegativeOverflow);

        CPPUNIT_TEST(testEmptyName);
        CPPUNIT_TEST(testSinglePrefix);
        CPPUNIT_TEST(testDoublePrefix);
        CPPUNIT_TEST(testBoolWithNoArgument);
        CPPUNIT_TEST(testUnrecognizedArgument);
        CPPUNIT_TEST(testOptionalValueAndUnrecognizedArgument);
        CPPUNIT_TEST(testNoShortValue);
        CPPUNIT_TEST(testNoLongValue);
        CPPUNIT_TEST(testInvalidValueType);
        CPPUNIT_TEST(testCustomHandlerShort);
        CPPUNIT_TEST(testCustomHandlerLong);
        CPPUNIT_TEST(testCustomNoValue);
        CPPUNIT_TEST(testCustomNoValueExpected);
        CPPUNIT_TEST(testInlineValues);
        CPPUNIT_TEST(testInlineValuesTypeMismatch);
        CPPUNIT_TEST(testRepeatingOptions_Int);
        CPPUNIT_TEST(testRepeatingOptionsCustomHandler);
        CPPUNIT_TEST(testRepeatingOptionsWithDifferentType);

        CPPUNIT_TEST(testMandatoryArgument);
        CPPUNIT_TEST(testMandatoryArgumentMissing);
        CPPUNIT_TEST(testMandatoryArgumentWithoutGivenFlags);
        CPPUNIT_TEST(testMandatoryArgumentOnly);
        CPPUNIT_TEST(testMandatoryArgumentNotEnough);
        CPPUNIT_TEST(testMandatoryArgumentTooMany);

        CPPUNIT_TEST(testCommandGivenButNotExpected);
        CPPUNIT_TEST(testMandatoryCommandNotGiven);
        CPPUNIT_TEST(testMandatoryCommandWithNoArgumentsSuccess);
        CPPUNIT_TEST(testInvalidCommand);
        CPPUNIT_TEST(testInvalidArgumentsForCommand);
        CPPUNIT_TEST(testUnexpectedArgumentsForCommand);

        CPPUNIT_TEST(multipleCommandSelection);
        CPPUNIT_TEST(multipleCommandWithSimilarOptions);
        CPPUNIT_TEST(commandExecutionFails);

        CPPUNIT_TEST(multipleCommandWithOptionsAndArguments);
    CPPUNIT_TEST_SUITE_END();

protected:

    template<typename T>
    void testIntParsing(const char* strArg, T expectedValue, bool expectedToPass = true) {
        T parsedValue { 0 };

        const char* argv[] = {"prog", "-x", strArg, nullptr};
        const auto result = Parser("Something awesome", {
                              {{"x", "xxx"}, "Something", &parsedValue}
                          })
                .parse(countArgc(argv), argv);

        CPPUNIT_ASSERT_EQUAL(static_cast<T>(expectedValue), parsedValue);
        CPPUNIT_ASSERT_EQUAL(expectedToPass, result ? true : false);
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

public:

    void parseNullString() {
        CPPUNIT_ASSERT(Parser("Something awesome")
                       .parse(0, nullptr)
                       .isOk());
    }

    void parseEmptyString() {
        const char* argv[] = {""};

        CPPUNIT_ASSERT(Parser("Something awesome")
                       .parse(0, argv)
                       .isOk());

        CPPUNIT_ASSERT(Parser("Something awesome")
                       .parse(1, argv)
                       .isOk());
    }


    void parseOneArgumentString() {
        const char* argv[] = {"blarg!"};

        CPPUNIT_ASSERT(Parser("Something awesome", {})
                       .parse(1, argv)
                       .isOk());
    }

    void parseNegativeNumberOfArgument() {
        const char* argv[] = {"blarg!"};

        CPPUNIT_ASSERT(Parser("Something awesome")
                       .parse(-31, argv)
                       .isError());
    }

    void parseInt8() {
        testIntParsing<int8>("120", 120);
    }

    void parseInt8_Overflow() {
        testIntOverflow<int8>("32042");
    }

    void parseUInt8() {
        testIntParsing<uint8>("240", 240);
    }

    void parseUInt8_Overflow() {
        testIntOverflow<uint8>("429883");
    }

    void parseUInt8_NegativeArgument() {
        testIntParsing<uint8>("-32", 0, false);
    }

    void parseUInt8_NegativeOverflow() {
        testIntParsing<uint8>("-739834887", 0, false);
    }


    void parseInt16() {
        testIntParsing<int16>("321", 321);
    }

    void parseInt16_Overflow() {
        testIntOverflow<int16>("68535");
    }

    void parseUInt16() {
        testIntParsing<uint16>("9883", 9883);
    }

    void parseUInt16_Overflow() {
        testIntOverflow<uint16>("429883");
    }

    void parseUInt16_NegativeArgument() {
        testIntParsing<uint16>("-73", 0, false);
    }

    void parseUInt16_NegativeOverflow() {
        testIntParsing<uint16>("-739834887", 0, false);
    }


    void parseInt32() {
        testIntParsing<int32>("717321", 717321);
    }

    void parseInt32_Overflow() {
        testIntOverflow<int32>("9898847598475");
    }

    void parseUInt32() {
        testIntParsing<uint32>("19587446", 19587446);
    }

    void parseUInt32_Overflow() {
        testIntOverflow<uint32>("4298833432");
    }

    void parseUInt32_NegativeArgument() {
        testIntParsing<uint32>("-19587446", 0, false);
    }

    void parseUInt32_NegativeOverflow() {
        testIntParsing<uint32>("-9898847598475", 0, false);
    }


    void parseInt64() {
        testIntParsing<int64>("717321", 717321);
    }

    void parseInt64_Overflow() {
        testIntOverflow<int64>("9898847598475978947899839987438957");
        testIntOverflow<int64>("922337203685477580742111");
    }

    void parseUInt64() {
        testIntParsing<uint64>("19587446", 19587446);
    }

    void parseUInt64_Overflow() {
        testIntOverflow<uint64>("9898847598475978947899839987438957");
        testIntOverflow<uint64>("92233720368547758072");
    }

    void parseUInt64_NegativeArgument() {
        testIntParsing<uint64>("-19587446", 0, false);
    }

    void parseUInt64_NegativeOverflow() {
        testIntParsing<uint64>("-922337203685477580712", 0, false);
    }


    void testBoolWithNoArgument() {
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

                        CPPUNIT_FAIL(e.toString().c_str());
                    });

        CPPUNIT_ASSERT(parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(true, xValue);
        CPPUNIT_ASSERT_EQUAL(321, vValue);
    }

    void testUnrecognizedArgument() {
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


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(756, xValue);
    }


    void testOptionalValueAndUnrecognizedArgument() {
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


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(true, xValue);
    }


    void testNoShortValue() {
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


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(0, xValue);
    }


    void testNoLongValue() {

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


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(0, xValue);
    }


    void testInvalidValueType() {

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


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(0, xValue);
    }


    void testEmptyName() {
        bool parsedSuccessully = false;
        int xValue = 0;

        const char* argv[] = {"prog", "", "-xy", "32", nullptr};
        const char* appDesc = "Something awesome";
        Parser(appDesc, {{{"x", "xxx"}, "Something", &xValue}})
                .parse(countArgc(argv), argv)
                .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error&&){parsedSuccessully = false;});


        CPPUNIT_ASSERT_EQUAL(0, xValue);
        CPPUNIT_ASSERT(!parsedSuccessully);
    }


    void testSinglePrefix() {
        bool parsedSuccessully = false;
        int xValue = 0;

        const char* argv[] = {"prog", "-", "32", nullptr};
        const char* appDesc = "Something awesome";
        Parser(appDesc, {{{"x", "xxx"}, "Something", &xValue}})
                .parse(countArgc(argv), argv)
                .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error&&){parsedSuccessully = false;});


        CPPUNIT_ASSERT_EQUAL(0, xValue);
        CPPUNIT_ASSERT(!parsedSuccessully);
    }


    void testDoublePrefix() {
        bool parsedSuccessully = false;
        int xValue = 0;

        const char* argv[] = {"prog", "--", "BHAL!", nullptr};
        const char* appDesc = "Something awesome";
        Parser(appDesc, {{{"x", "xxx"}, "Something", &xValue}})
                .parse(countArgc(argv), argv)
                .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error&&){parsedSuccessully = false;});


        CPPUNIT_ASSERT_EQUAL(0, xValue);
        CPPUNIT_ASSERT(!parsedSuccessully);
    }


    void testCustomHandlerLong() {
        bool parsedSuccessully = false;
        bool customCalled = false;
        int xValue = 0;
        const char* zValue = nullptr;
        const char* zExpValue = "somethin";

        const char* argv[] = {"prog", "--xxx", "756", "--zva", "somethin", nullptr};
        const char* appDesc = "Something awesome";
        Parser(appDesc, {
                      {{"x", "xxx"}, "Something", &xValue},
                      {{"z", "zva"}, "Custom arg", Parser::OptionArgument::Required,
                       [&customCalled, &zValue](const Optional<StringView>& value, const Parser::Context&) {
                           customCalled = true;
                           zValue = value.get().c_str();

                           return None();
                       }}
                  })
                .parse(countArgc(argv), argv)
                .then([&parsedSuccessully](Parser::ParseResult&&) { parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error&&) { parsedSuccessully = false; });


        CPPUNIT_ASSERT(parsedSuccessully);
        CPPUNIT_ASSERT(customCalled);
        CPPUNIT_ASSERT_EQUAL(756, xValue);
        CPPUNIT_ASSERT_EQUAL(zExpValue, zValue);
    }


    void testCustomHandlerShort() {
        bool parsedSuccessully = false;
        bool customCalled = false;
        int xValue = 0;
        const char* zValue = nullptr;
        const char* zExpValue = "somethin2";

        const char* argv[] = {"prog", "--xxx", "756", "-z", "somethin2", nullptr};
        const char* appDesc = "Something awesome";
        Parser(appDesc, {
                      {{"x", "xxx"}, "Something", &xValue},
                      {{"z", "zve"}, "Custom arg", Parser::OptionArgument::Required,
                       [&customCalled, &zValue](const Optional<StringView>& value, const Parser::Context&) {
                           customCalled = true;
                           zValue = value.get().c_str();

                           return None();
                       }}
                  })
                .parse(countArgc(argv), argv)
                .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error){parsedSuccessully = false;});


        CPPUNIT_ASSERT(parsedSuccessully);
        CPPUNIT_ASSERT(customCalled);
        CPPUNIT_ASSERT_EQUAL(756, xValue);
        CPPUNIT_ASSERT_EQUAL(zExpValue, zValue);
    }


    void testCustomNoValue() {
        bool parsedSuccessully = false;
        bool customCalled = false;
        int xValue = 0;
        const char* zValue = nullptr;

        const char* argv[] = {"prog", "--xxx", "756", "-z", nullptr};
        const char* appDesc = "Something awesome";
        Parser(appDesc, {
                      {{"x", "xxx"}, "Something", &xValue},
                      {{"z", "zve"}, "Custom arg", Parser::OptionArgument::Required,
                       [&customCalled, &zValue](const Optional<StringView>& value, const Parser::Context&) {
                           customCalled = true;
                           zValue = value.get().c_str();

                           return None();
                       }}
                  })
                .parse(countArgc(argv), argv)
                .then([&parsedSuccessully](Parser::ParseResult&&) { parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT(!customCalled);
        CPPUNIT_ASSERT_EQUAL(756, xValue);
    }


    void testCustomNoValueExpected() {
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

                                   return None();
                               } }
                          })
                .parse(countArgc(argv), argv)
                .then([&parsedSuccessully](Parser::ParseResult&&) { parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


        CPPUNIT_ASSERT(parsedSuccessully);
        CPPUNIT_ASSERT(customCalled);
        CPPUNIT_ASSERT_EQUAL(756, xValue);
    }

    void testInlineValues() {
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

                        CPPUNIT_FAIL(e.toString().c_str());
                    });

        CPPUNIT_ASSERT(parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(true, xValue);
        CPPUNIT_ASSERT_EQUAL(321, vValue);
        CPPUNIT_ASSERT(sValue == "blah!");
    }

    void testInlineValuesTypeMismatch() {
        int vValue = 0;
        StringView sValue;

        const char* argv[] = {"prog", "--intValue=Hello", nullptr};
        const auto result = Parser("Something awesome", {
                              {{"intValue"}, "Int Value", &vValue},
                              {{"s", "string"}, "String value", &sValue}
                          })
                .parse(countArgc(argv), argv);

        CPPUNIT_ASSERT(result.isError());
    }


    void testRepeatingOptions_Int() {
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

                        CPPUNIT_FAIL(e.toString().c_str());
                    });

        CPPUNIT_ASSERT(parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(918, vValue);
    }

    void testRepeatingOptionsCustomHandler() {
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
                               [&vValue](const Optional<StringView>& value, const Parser::Context&) {
                                   auto res = tryParse<int>(value.get());
                                   if (res) {
                                        vValue += res.unwrap();
                                        return Optional<Error>::none();
                                   } else {
                                       return Optional<Error>::of(res.moveError());
                                   }
                               }},
                              {{"v"}, "Useless value", &unusedValue}
                          })
                .parse(countArgc(argv), argv)
                .then([&parsedSuccessully](Parser::ParseResult&&) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error&& e) {
                        parsedSuccessully = false;

                        CPPUNIT_FAIL(e.toString().c_str());
                    });

        CPPUNIT_ASSERT(parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(918 + 32 + 17, vValue);
    }

    void testRepeatingOptionsWithDifferentType() {
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

        CPPUNIT_ASSERT(result.isError());
    }


    void testMandatoryArgument() {
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


        CPPUNIT_ASSERT(parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(756, xValue);
        CPPUNIT_ASSERT_EQUAL(98765, mandatoryArg);
    }


    void testMandatoryArgumentOnly() {
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


        CPPUNIT_ASSERT(parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(StringView("awesome-value"), mandatoryArg);
    }

    void testMandatoryArgumentWithoutGivenFlags() {
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


        CPPUNIT_ASSERT(parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(0, xValue);
        CPPUNIT_ASSERT_EQUAL(StringView("awesome-value"), mandatoryArg);
    }


    void testMandatoryArgumentMissing() {
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


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(0, mandatoryArg);
        CPPUNIT_ASSERT_EQUAL(756, xValue);
    }


    void testMandatoryArgumentNotEnough() {
        StringView mandatoryArgStr;
        int mandatoryArgInt = 0;
        int mandatoryArgInt2 = 0;

        const char* argv[] = {"prog", "do", "321", nullptr};
        const auto result = Parser("Something awesome")
                .arguments({
                    {"manarg1", "Mandatory argument", &mandatoryArgStr},
                    {"manarg2", "Mandatory argument", &mandatoryArgInt},
                    {"manarg3", "Mandatory argument", &mandatoryArgInt2}
                })
                .parse(countArgc(argv), argv);

        CPPUNIT_ASSERT(result.isError());
    }


    void testMandatoryArgumentTooMany() {
        StringView mandatoryArgStr;
        int mandatoryArgInt = 0;

        const char* argv[] = {"prog", "some", "756", "other", nullptr};
        const auto result = Parser("Something awesome")
                .arguments({
                    {"manarg1", "Mandatory argument", &mandatoryArgStr},
                    {"manarg2", "Mandatory argument", &mandatoryArgInt}
                })
                .parse(countArgc(argv), argv);

        CPPUNIT_ASSERT(result.isError());
    }


    void testCommandGivenButNotExpected() {
        bool commandExecuted = false;
        bool givenOpt = false;

        const char* argv[] = {"prog", "command", nullptr};
        const auto result = Parser("Something awesome",
                            {{{"b", "bsome"}, "Some option", &givenOpt}
                          })
                .parse(countArgc(argv), argv);


        CPPUNIT_ASSERT(result.isError());
        CPPUNIT_ASSERT(!commandExecuted);
    }


    void testMandatoryCommandNotGiven() {
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

        CPPUNIT_ASSERT(!commandExecuted);
        CPPUNIT_ASSERT(result.isError());
    }

    void testMandatoryCommandWithNoArgumentsSuccess() {
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
            CPPUNIT_FAIL(result.getError().toString().c_str());
        } else {
            CPPUNIT_ASSERT(result.unwrap()().isOk());
        }

        CPPUNIT_ASSERT(commandExecuted);
    }

    void testInvalidCommand() {
        bool commandExecuted = false;

        const char* argv[] = {"prog", "somethingElse", nullptr};
        const auto result = Parser("Something awesome")
                .commands({
                              {"doIt", {"Pass the test",
                               [&commandExecuted]() -> Result<void, Error> {
                                   commandExecuted = true;
                                   return Ok();
                               }}}
                          })
                .parse(countArgc(argv), argv);

        CPPUNIT_ASSERT(result.isError());
        CPPUNIT_ASSERT(!commandExecuted);
    }


    void testInvalidArgumentsForCommand() {
        bool commandExecuted = false;


        struct CmdCntx {
            bool all;
        } cmdCntx;

        const char* argv[] = {"prog", "doIt", "b", "blah!", nullptr};
        const auto result = Parser("Something awesome")
                .commands({
                              {"doIt", {"Pass the test",
                               [&commandExecuted]() -> Result<void, Error> {
                                   commandExecuted = true;
                                   return Ok();
                               }, {
                                   {{"a", "all"}, "Do something everywhere", &cmdCntx.all}
                               }}}
                          })
                .parse(countArgc(argv), argv);

        CPPUNIT_ASSERT(result.isError());
        CPPUNIT_ASSERT(!commandExecuted);
    }

    void testUnexpectedArgumentsForCommand() {
        bool commandExecuted = false;

        struct CmdCntx {
            bool all;
        } cmdCntx;

        const char* argv[] = {"prog", "doIt", "-a", "-b", "blah!", nullptr};
        const auto result = Parser("Something awesome")
                .commands({
                              {"doIt", {"Pass the test",
                               [&commandExecuted]() -> Result<void, Error> {
                                   commandExecuted = true;

                                   return Ok();
                               }, {
                                   {{"a", "all"}, "Do something everywhere", &cmdCntx.all}
                               }}}
                          })
                .parse(countArgc(argv), argv);

        CPPUNIT_ASSERT(result.isError());
        CPPUNIT_ASSERT(!commandExecuted);
    }

    void multipleCommandSelection() {
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
            CPPUNIT_FAIL(parseResult.getError().toString().c_str());
        } else {
            CPPUNIT_ASSERT(parseResult.unwrap()().isOk());
        }

        CPPUNIT_ASSERT(commandExecuted[0]);
        CPPUNIT_ASSERT(!commandExecuted[1]);
    }

    void multipleCommandWithSimilarOptions() {
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
            CPPUNIT_FAIL(result.getError().toString().c_str());
        } else {
            CPPUNIT_ASSERT(result.unwrap()().isOk());
        }

        CPPUNIT_ASSERT(!commandExecuted[0]);
        CPPUNIT_ASSERT(commandExecuted[1]);
        CPPUNIT_ASSERT_EQUAL(321, cmd2Options.commonFlag);
        CPPUNIT_ASSERT(!commandExecuted[2]);
    }

    void commandExecutionFails() {
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
            CPPUNIT_FAIL(result.getError().toString().c_str());
        } else {
            CPPUNIT_ASSERT(result.unwrap()().isError());
        }

        CPPUNIT_ASSERT(!commandExecuted[0]);
        CPPUNIT_ASSERT(commandExecuted[1]);
    }


    void multipleCommandWithOptionsAndArguments() {
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

                              {"comm-2", {"Run 2nd command",
                               [&]() -> Result<void, Error> {
                                   commandExecuted[1] = true;
                                   return Ok();
                               }, {
                                   {{"c", "commonOption"}, "Common option", &cmd2Options.commonFlag},
                                   {{"o", "uniqueOption"}, "Some unique option", &cmd2Options.value}
                               }, {
                                   {"arg1", "Arg value1", &cmd2Options.arg1},
                                   {"arg2", "Arg value2", &cmd2Options.arg2}

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
            CPPUNIT_FAIL(result.getError().toString().c_str());
        } else {
            CPPUNIT_ASSERT(result.unwrap()().isOk());
        }

        CPPUNIT_ASSERT(!commandExecuted[0]);
        CPPUNIT_ASSERT(commandExecuted[1]);
        CPPUNIT_ASSERT(!commandExecuted[2]);


        CPPUNIT_ASSERT_EQUAL(true, verbose);
        CPPUNIT_ASSERT_EQUAL(42, globalInt);

        CPPUNIT_ASSERT_EQUAL(11, cmd2Options.value);
        CPPUNIT_ASSERT_EQUAL(StringView("ArgValue1"), cmd2Options.arg1);
        CPPUNIT_ASSERT_EQUAL(StringView("arg2"), cmd2Options.arg2);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCommandlineParser);

