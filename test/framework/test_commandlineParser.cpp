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
        CPPUNIT_TEST(testShortInt);
        CPPUNIT_TEST(testLongInt);
        CPPUNIT_TEST(testUnrecogniezedArgument);
        CPPUNIT_TEST(testNoShortValue);
        CPPUNIT_TEST(testNoLongValue);
        CPPUNIT_TEST(testInvalidValueType);
        CPPUNIT_TEST(testCustomHandlerShort);
        CPPUNIT_TEST(testCustomHandlerLong);
        CPPUNIT_TEST(testCustomNoValue);

        CPPUNIT_TEST(testMandatoryArgument);
        CPPUNIT_TEST(testMandatoryArgumentMissing);

    CPPUNIT_TEST_SUITE_END();

public:


    void testShortInt() {

        bool parsedSuccessully = false;
        int xValue = 0;

        const char* argv[] = {"prog", "-x", "321", nullptr};
        const int argc = 3;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {
                              {'x', "xxx", "Something", &xValue}
                          })
                .parse(argc, argv)
                .then<void>(
                    [&parsedSuccessully](Unit) {parsedSuccessully = true; },
                    [&parsedSuccessully](Error){parsedSuccessully = false;});


        CPPUNIT_ASSERT_EQUAL(321, xValue);
        CPPUNIT_ASSERT(parsedSuccessully);
    }


    void testLongInt() {

        bool parsedSuccessully = false;
        int xValue = 0;

        const char* argv[] = {"prog", "--xxx", "756", nullptr};
        const int argc = 3;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {
                              {'x', "xxx", "Something", &xValue}
                          })
                .parse(argc, argv)
                .then<void>(
                    [&parsedSuccessully](Unit) {parsedSuccessully = true; },
                    [&parsedSuccessully](Error){parsedSuccessully = false;});


        CPPUNIT_ASSERT_EQUAL(756, xValue);
        CPPUNIT_ASSERT(parsedSuccessully);
    }


    void testUnrecogniezedArgument() {

        bool parsedSuccessully = false;
        int xValue = 0;

        const char* argv[] = {"prog", "--xxx", "756", "--unknown", nullptr};
        const int argc = 4;

        const char* appDesc = "Something awesome";
        CommandlineParser(appDesc, {
                              {'x', "xxx", "Something", &xValue}
                          })
                .parse(argc, argv)
                .then<void>(
                    [&parsedSuccessully](Unit) {parsedSuccessully = true; },
                    [&parsedSuccessully](Error){parsedSuccessully = false;});


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(756, xValue);
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
                .then<void>(
                    [&parsedSuccessully](Unit) {parsedSuccessully = true; },
                    [&parsedSuccessully](Error){parsedSuccessully = false;});


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
                .then<void>(
                    [&parsedSuccessully](Unit) {parsedSuccessully = true; },
                    [&parsedSuccessully](Error){parsedSuccessully = false;});


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
                .then<void>(
                    [&parsedSuccessully](Unit) {parsedSuccessully = true; },
                    [&parsedSuccessully](Error){parsedSuccessully = false;});


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
                              {'z', "zva", "Custom arg", [&customCalled, &zValue](const char* value) {
                                   customCalled = true;
                                   zValue = value;

                                   return None();
                               }}
                          })
                .parse(argc, argv)
                .then<void>(
                    [&parsedSuccessully](Unit) {parsedSuccessully = true; },
                    [&parsedSuccessully](Error){parsedSuccessully = false;});


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
                              {'z', "zve", "Custom arg", [&customCalled, &zValue](const char* value) {
                                   customCalled = true;
                                   zValue = value;

                                   return None();
                               }}
                          })
                .parse(argc, argv)
                .then<void>(
                    [&parsedSuccessully](Unit) {parsedSuccessully = true; },
                    [&parsedSuccessully](Error){parsedSuccessully = false;});


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
                              {'z', "zve", "Custom arg", [&customCalled, &zValue](const char* value) {
                                   customCalled = true;
                                   zValue = value;

                                   return None();
                               }}
                          })
                .parse(argc, argv)
                .then<void>(
                    [&parsedSuccessully](Unit) {parsedSuccessully = true; },
                    [&parsedSuccessully](Error){parsedSuccessully = false;});


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT(!customCalled);
        CPPUNIT_ASSERT_EQUAL(756, xValue);
    }


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
                              {"Mandatory argument", &mandatoryArg}
                          })
                .parse(argc, argv)
                .then<void>(
                    [&parsedSuccessully](Unit) {parsedSuccessully = true; },
                    [&parsedSuccessully](Error){parsedSuccessully = false;});


        CPPUNIT_ASSERT(parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(756, xValue);
        CPPUNIT_ASSERT_EQUAL(98765, mandatoryArg);
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
                              {"Mandatory argument", &mandatoryArg}
                          }
                          )
                .parse(argc, argv)
                .then<void>(
                    [&parsedSuccessully](Unit) {parsedSuccessully = true; },
                    [&parsedSuccessully](Error){parsedSuccessully = false;});


        CPPUNIT_ASSERT(!parsedSuccessully);
        CPPUNIT_ASSERT_EQUAL(0, mandatoryArg);
        CPPUNIT_ASSERT_EQUAL(756, xValue);
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCommandlineParser);

