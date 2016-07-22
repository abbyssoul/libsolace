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
 *  @file     test/main.cpp
 *  @author   $LastChangedBy: $
 *  @date     $LastChangedDate: $
 *  @brief    Unit Tests entry point
  * ID:       $id$
 ******************************************************************************/
#include <iostream>
#include <stdexcept>

#include <cppunit/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/Protector.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>

#include <memory>


#include "ci/teamcity_cppunit.h"
#include "interruptexception.hpp"


class IntrruptableTestRunner {
public:

    virtual ~IntrruptableTestRunner() = default;

    virtual void stop() {
        _isStopped = true;
    }

    bool isStopped() const {
        return _isStopped;
    }

private:
    bool _isStopped = false;
};

class TestStopper: public CppUnit::Protector {
public:

    TestStopper(IntrruptableTestRunner& testRunner): _testRunner(testRunner) {
    }

    // cppcheck-suppress unusedFunction
    bool protect(const CppUnit::Functor &functor, const CppUnit::ProtectorContext&) override {

        try {
            return functor();
        } catch (InterruptTest& allCool) {  // Note: This is normal test case interruption possibly by a child process
            _testRunner.stop();
            return true;
        }

        return false;
    }

private:
    IntrruptableTestRunner& _testRunner;
};


class TestRunner: public IntrruptableTestRunner {
public:

    static std::unique_ptr<CppUnit::TestListener> createProgressListener() {

        if (jetbrains::teamcity::underTeamcity()) {
            // Add unique flowId parameter if you want to run test processes in parallel
            // See http://confluence.jetbrains.net/display/TCD6/Build+Script+Interaction+with+TeamCity#BuildScriptInteractionwithTeamCity-MessageFlowId
            return std::make_unique<jetbrains::teamcity::TeamcityProgressListener>();
        } else {
            return std::make_unique<CppUnit::BriefTestProgressListener>();
//            return std::make_unique<CppUnit::TextTestProgressListener>();
        }
    }

    TestRunner(): progressListener(createProgressListener()) {
        // Wire it all up
        controller.addListener(&result);

        if (progressListener) {
            controller.addListener(progressListener.get());
        }

        controller.pushProtector(new TestStopper(*this));
    }

    ~TestRunner() = default;

    void stop() override {
        IntrruptableTestRunner::stop();
        controller.stop();
    }

    TestRunner& scanTests() {
        auto& registry = CppUnit::TestFactoryRegistry::getRegistry();
        runner.addTest(registry.makeTest());

        return *this;
    }

    int run(const char* path) {

        testPath = path;
        try {
            runner.run(controller, testPath);

            if (!isStopped()) {
                // Print test in a compiler compatible format.
                CppUnit::CompilerOutputter outputter(&result, std::cerr, "%f:%l: ");
                outputter.write();
            }

        } catch (std::invalid_argument &e) {  // Test path not resolved
            std::cerr << std::endl << "ERROR: Test path not resolved: " << e.what() << std::endl;

            return 2;
        }

        return result.wasSuccessful() ? 0 : 1;
    }

private:
    std::string testPath;

    // Add a listener that print dots as test run.
    std::unique_ptr<CppUnit::TestListener> progressListener;

    // Create the event manager and test controller
    CppUnit::TestResult controller;

    // Add a listener that collects test result
    CppUnit::TestResultCollector result;

    // Add the top suite to the test runner
    CppUnit::TextUi::TestRunner runner;
};


// Note: Test runner made global in order to invoke it's destructor when fork'd version of it does exit()
TestRunner GlobalTestRunner;

int main(int argc, char* argv[]) {
    // FIXME(abbyssoul): Add signal handling in test ::signal(SIGSEGV, _sighandler);
    srandom(time(NULL));

    return GlobalTestRunner.scanTests().run((argc > 1) ? argv[1] : "");
}
