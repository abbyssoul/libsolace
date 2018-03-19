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
 * @file: test/framework/test_commandlineParser.cpp
 * @author: soultaker
 *
 * Created on: 20/06/2016
*******************************************************************************/
#include <solace/framework/commandlineParser.hpp>  // Class being tested

#include <cppunit/extensions/HelperMacros.h>


using namespace Solace;
using namespace Solace::Framework;


class TestCommandlineParser: public CppUnit::TestFixture  {

    CPPUNIT_TEST_SUITE(TestCommandlineParser);
        CPPUNIT_TEST(parseNullString);
        CPPUNIT_TEST(parseEmptyString);
        CPPUNIT_TEST(parseOneArgumentString);

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

        CPPUNIT_TEST(testBoolWithNoArgument);
        CPPUNIT_TEST(testUnrecognizedArgument);
        CPPUNIT_TEST(testOptionalValueAndUnrecognizedArgument);
        CPPUNIT_TEST(testNoShortValue);
        CPPUNIT_TEST(testNoLongValue);
        CPPUNIT_TEST(testInvalidValueType);
        CPPUNIT_TEST(testEmptyName);
        CPPUNIT_TEST(testSinglePrefix);
        CPPUNIT_TEST(testDoublePrefix);
        CPPUNIT_TEST(testCustomHandlerShort);
        CPPUNIT_TEST(testCustomHandlerLong);
        CPPUNIT_TEST(testCustomNoValue);
        CPPUNIT_TEST(testCustomNoValueExpected);

//        CPPUNIT_TEST(testMandatoryArgument);
//        CPPUNIT_TEST(testMandatoryArgumentMissing);
//        CPPUNIT_TEST(testMandatoryArgumentWithoutGivenFlags);
//        CPPUNIT_TEST(testMandatoryArgumentOnly);
//        CPPUNIT_TEST(testMandatoryArgumentNotEnought);
//        CPPUNIT_TEST(testMandatoryArgumentTooMany);

        CPPUNIT_TEST(testCommandGivenButNotExpected);
        CPPUNIT_TEST(testMandatoryCommandNotGiven);
        CPPUNIT_TEST(testMandatoryCommandWithNoArgumentsSuccess);
        CPPUNIT_TEST(testMandatoryCommandWithNoArguments_InvalidCommand);
        CPPUNIT_TEST(testMandatoryCommandInvalidArguments);

    CPPUNIT_TEST_SUITE_END();

protected:

    template<typename T>
    void testIntParsing(const char* strArg, T expectedValue, bool expectedToPass = true) {
        bool parsedSuccessully = false;
        T xValue = 0;

        const char* argv[] = {"prog", "-x", strArg, nullptr};
        const int argc = 3;

        CommandlineParser("Something awesome", {
                              {'x', "xxx", "Something", &xValue}
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) { parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error) { parsedSuccessully = false;});


        CPPUNIT_ASSERT_EQUAL(static_cast<T>(expectedValue), xValue);
        CPPUNIT_ASSERT_EQUAL(expectedToPass, parsedSuccessully);
    }

    template<typename T>
    void testIntOverflow(const char* strArg) {
        testIntParsing<T>(strArg, 0, false);
    }


public:

    void parseNullString() {
        CommandlineParser parser("Something awesome");
        auto result = parser.parse(0, nullptr);

        CPPUNIT_ASSERT(result.isOk());
    }

    void parseEmptyString() {
        const char* argv[] = {""};
        CommandlineParser parser("Something awesome");
        auto result1 = parser.parse(0, argv);

        CPPUNIT_ASSERT(result1.isOk());

        auto result2 = parser.parse(1, argv);

        CPPUNIT_ASSERT(result2.isOk());
    }


    void parseOneArgumentString() {
        const char* argv[] = {"blarg!"};

        CPPUNIT_ASSERT(CommandlineParser("Something awesome", {})
                       .parse(1, argv)
                       .isOk());
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
        const int argc = 4;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {
                              {'x', "xxx", "Something", &xValue},
                              {'V', "vvv", "Something else", &vValue}
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error e) {
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
        const int argc = 4;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {
                              CommandlineParser::Option{'x', "xxx", "Something", &xValue}
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error&&){parsedSuccessully = false;});


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(756, xValue);
    }


    void testOptionalValueAndUnrecognizedArgument() {
        bool parsedSuccessully = false;
        bool xValue = false;

        const char* argv[] = {"prog", "-v", "--unknown", "blah!", nullptr};
        const int argc = 4;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {
                              {'v', "vvv", "Something", &xValue}
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error&&) {parsedSuccessully = false;});


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(true, xValue);
    }


    void testNoShortValue() {

        bool parsedSuccessully = false;
        int xValue = 0;

        const char* argv[] = {"prog", "-x", nullptr};
        const int argc = 2;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {
                              {'x', "xxx", "Something", &xValue}
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error&&) {parsedSuccessully = false;});


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(0, xValue);
    }


    void testNoLongValue() {

        bool parsedSuccessully = false;
        int xValue = 0;

        const char* argv[] = {"prog", "--xxx", nullptr};
        const int argc = 2;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {
                              {'x', "xxx", "Something", &xValue}
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error&&){parsedSuccessully = false;});


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(0, xValue);
    }


    void testInvalidValueType() {

        bool parsedSuccessully = false;
        int xValue = 0;

        const char* argv[] = {"prog", "--xxx", "BHAL!", nullptr};
        const int argc = 3;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {
                              {'x', "xxx", "Something", &xValue}
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error){parsedSuccessully = false;});


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(0, xValue);
    }


    void testEmptyName() {
        bool parsedSuccessully = false;
        int xValue = 0;

        const char* argv[] = {"prog", "", "-xy", "32", nullptr};
        const int argc = 4;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {{'x', "xxx", "Something", &xValue}})
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error){parsedSuccessully = false;});


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(0, xValue);
    }


    void testSinglePrefix() {
        bool parsedSuccessully = false;
        int xValue = 0;

        const char* argv[] = {"prog", "-", "32", nullptr};
        const int argc = 3;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {{'x', "xxx", "Something", &xValue}})
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error){parsedSuccessully = false;});


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(0, xValue);
    }


    void testDoublePrefix() {
        bool parsedSuccessully = false;
        int xValue = 0;

        const char* argv[] = {"prog", "--", "BHAL!", nullptr};
        const int argc = 3;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {{'x', "xxx", "Something", &xValue}})
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error){parsedSuccessully = false;});


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(0, xValue);
    }


    void testCustomHandlerLong() {

        bool parsedSuccessully = false;
        bool customCalled = false;
        int xValue = 0;
        const char* zValue = nullptr;
        const char* zExpValue = "somethin";

        const char* argv[] = {"prog", "--xxx", "756", "--zva", "somethin", nullptr};
        const int argc = 5;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {
                              {'x', "xxx", "Something", &xValue},
                              {'z', "zva", "Custom arg", CommandlineParser::OptionArgument::Required,
                               [&customCalled, &zValue](CommandlineParser::Context& c) {
                                   customCalled = true;
                                   zValue = c.value;

                                   return None();
                               }}
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error){parsedSuccessully = false;});


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
        const int argc = 5;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {
                              {'x', "xxx", "Something", &xValue},
                              {'z', "zve", "Custom arg", CommandlineParser::OptionArgument::Required,
                               [&customCalled, &zValue](CommandlineParser::Context& c) {
                                   customCalled = true;
                                   zValue = c.value;

                                   return None();
                               }}
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) {parsedSuccessully = true; })
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
        const int argc = 4;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {
                              {'x', "xxx", "Something", &xValue},
                              {'z', "zve", "Custom arg", CommandlineParser::OptionArgument::Required
                               , [&customCalled, &zValue](CommandlineParser::Context& c) {
                                   customCalled = true;
                                   zValue = c.value;

                                   return None();
                               }}
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) { parsedSuccessully = true; })
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
        const int argc = 4;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {
                              {'x', "xxx", "Something", &xValue},
                              {'z', "zve", "Custom arg", CommandlineParser::OptionArgument::NotRequired,
                               [&customCalled](CommandlineParser::Context&) {
                                   customCalled = true;

                                   return None();
                               } }
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) { parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


        CPPUNIT_ASSERT(parsedSuccessully);
        CPPUNIT_ASSERT(customCalled);
        CPPUNIT_ASSERT_EQUAL(756, xValue);
    }
/*
    void testMandatoryArgument() {

        bool parsedSuccessully = false;
        int xValue = 0;
        int mandatoryArg = 0;

        const char* argv[] = {"prog", "-x", "756", "98765", nullptr};
        const int argc = 4;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {
                              {'x', "xxx", "Something", &xValue}
                          }, {
                              {"manarg", "Mandatory argument", &mandatoryArg}
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) { parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error) { parsedSuccessully = false;});


        CPPUNIT_ASSERT(parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(756, xValue);
        CPPUNIT_ASSERT_EQUAL(98765, mandatoryArg);
    }


    void testMandatoryArgumentOnly() {
        bool parsedSuccessully = false;
        String mandatoryArg;

        const char* argv[] = {"prog", "awesome-value", nullptr};
        const int argc = 2;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {},
                            {
                              {"manarg", "Mandatory argument", &mandatoryArg}
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) { parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


        CPPUNIT_ASSERT(parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(String("awesome-value"), mandatoryArg);
    }

    void testMandatoryArgumentWithoutGivenFlags() {
        bool parsedSuccessully = false;
        int xValue = 0;
        String mandatoryArg;

        const char* argv[] = {"prog", "awesome-value", nullptr};
        const int argc = 2;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {
                              {'x', "xxx", "Something", &xValue}
                          }, {
                              {"manarg", "Mandatory argument", &mandatoryArg}
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) { parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


        CPPUNIT_ASSERT(parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(0, xValue);
        CPPUNIT_ASSERT_EQUAL(String("awesome-value"), mandatoryArg);
    }


    void testMandatoryArgumentMissing() {
        bool parsedSuccessully = false;
        int xValue = 0;
        int mandatoryArg = 0;

        const char* argv[] = {"prog", "-x", "756", nullptr};
        const int argc = 3;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {
                              {'x', "xxx", "Something", &xValue}
                          }, {
                              {"manarg", "Mandatory argument", &mandatoryArg}
                          }
                          )
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) { parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(0, mandatoryArg);
        CPPUNIT_ASSERT_EQUAL(756, xValue);
    }


    void testMandatoryArgumentNotEnought() {
        bool parsedSuccessully = false;

        String mandatoryArgStr;
        int mandatoryArgInt = 0;
        int mandatoryArgInt2 = 0;

        const char* argv[] = {"prog", "do", "321", nullptr};
        const int argc = 3;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {}, {
                              {"manarg1", "Mandatory argument", &mandatoryArgStr},
                              {"manarg2", "Mandatory argument", &mandatoryArgInt},
                              {"manarg3", "Mandatory argument", &mandatoryArgInt2}
                          }
                          )
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) { parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(0, mandatoryArgInt);
    }


    void testMandatoryArgumentTooMany() {
        bool parsedSuccessully = false;

        String mandatoryArgStr;
        int mandatoryArgInt = 0;

        const char* argv[] = {"prog", "some", "756", "other", nullptr};
        const int argc = 4;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {}, {
                              {"manarg1", "Mandatory argument", &mandatoryArgStr},
                              {"manarg2", "Mandatory argument", &mandatoryArgInt}
                          }
                          )
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT(mandatoryArgStr.empty());
        CPPUNIT_ASSERT_EQUAL(0, mandatoryArgInt);
    }
*/

    void testCommandGivenButNotExpected() {
        bool parsedSuccessully = false;
        bool commandExecuted = false;
        bool givenOpt = false;

        const char* argv[] = {"prog", "command", nullptr};
        const int argc = 2;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {{'b', "bsome", "Some option", &givenOpt}})
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) { parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT(!commandExecuted);
    }


    void testMandatoryCommandNotGiven() {
        bool parsedSuccessully = false;
        bool commandExecuted = false;

        const char* argv[] = {"prog", nullptr};
        const int argc = 1;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc)
                .commands({
                              {"doThings", "Mandatory command",
                               [&commandExecuted](CommandlineParser::Context&) -> Optional<Error> {
                                   commandExecuted = true;

                                   return None();
                               }
                              }
                        })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) { parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


        CPPUNIT_ASSERT(!commandExecuted);
        CPPUNIT_ASSERT(!parsedSuccessully);
    }

    void testMandatoryCommandWithNoArgumentsSuccess() {
        bool parsedSuccessully = false;
        bool commandExecuted = false;

        const char* argv[] = {"prog", "doIt", nullptr};
        const int argc = 2;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc)
                .commands({
                              {"doIt", "Pass the test",
                               [&commandExecuted](CommandlineParser::Context&) -> Optional<Error> {
                                   commandExecuted = true;
                                   return None();
                               }, {}}
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


        CPPUNIT_ASSERT(parsedSuccessully);
        CPPUNIT_ASSERT(commandExecuted);
    }

    void testMandatoryCommandWithNoArguments_InvalidCommand() {
        bool parsedSuccessully = false;
        bool commandExecuted = false;

        const char* argv[] = {"prog", "somethingElse", nullptr};
        const int argc = 2;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc)
                .commands({
                              {"doIt", "Pass the test",
                               [&commandExecuted](CommandlineParser::Context&) -> Optional<Error> {
                                   commandExecuted = true;
                                   return None();
                               }, {}}
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT(!commandExecuted);
    }


    void testMandatoryCommandInvalidArguments() {
        bool parsedSuccessully = false;
        bool commandExecuted = false;

        const char* argv[] = {"prog", "somethingElse", "b", "blah!", nullptr};
        const int argc = 4;

        struct CmdCntx {
            bool all;
        } cmdCntx;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc)
                .commands({
                              {"doIt", "Pass the test",
                               [&commandExecuted](CommandlineParser::Context&) -> Optional<Error> {
                                   commandExecuted = true;
                                   return None();
                               }, {
                                   {'a', "all", "Do something everywhere", &cmdCntx.all}
                               }}
                          })
                .parse(argc, argv)
                .then([&parsedSuccessully](const CommandlineParser*) {parsedSuccessully = true; })
                .orElse([&parsedSuccessully](Error) { parsedSuccessully = false; });


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT(!commandExecuted);
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCommandlineParser);

