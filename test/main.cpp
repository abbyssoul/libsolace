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
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include <cppunit/ui/text/TestRunner.h>

#include <cppunit/CompilerOutputter.h>

#include <signal.h>
#include <cstdlib>
#include <memory>


#include "ci/teamcity_cppunit.h"


// void _sighandler(int sig) {
//   switch (sig) {
//   case SIGSEGV: { // Segmentation fault - for debug.

//     // print out all the frames to stderr
//     fprintf(stderr, "\nError - signal %d:\n", sig);

//     April::System::Debug::stacktrace_print(stderr, 40, 0);

//     // exit(System::Processes::Process::ExitStatus::SigSegmentationFault);
//     // FIXME: Create Core Dump.
//     abort();
//   } break;
// //  case SIGINT:    // Program correct end request via keyboard int (Ctrl+C)
// //  case SIGTERM:   // Program correct end request.
// //    break;
//   }
// }


int main(int argc, char* argv[]) {
    // FIXME(abbyssoul): Add signal handling in test ::signal(SIGSEGV, _sighandler);
    const std::string testPath = (argc > 1) ? std::string(argv[1]) : "";

    srandom(time(NULL));

    // Add a listener that print dots as test run.
    std::unique_ptr<CppUnit::TestListener> progressListener;
//    std::unique_ptr<CppUnit::TestListener> extraProgressListener;

    // Create the event manager and test controller
    CppUnit::TestResult controller;

    // Add a listener that collects test result
    CppUnit::TestResultCollector result;
    controller.addListener(&result);

    if (jetbrains::teamcity::underTeamcity()) {
        // Add unique flowId parameter if you want to run test processes in parallel
        // See http://confluence.jetbrains.net/display/TCD6/Build+Script+Interaction+with+TeamCity#BuildScriptInteractionwithTeamCity-MessageFlowId
        progressListener = std::make_unique<jetbrains::teamcity::TeamcityProgressListener>();
    } else {
//        progressListener = std::make_unique<CppUnit::BriefTestProgressListener>();
        progressListener = std::make_unique<CppUnit::TextTestProgressListener>();
    }
    controller.addListener(progressListener.get());

    // Add the top suite to the test runner
    CppUnit::TextUi::TestRunner runner;
    auto& registry = CppUnit::TestFactoryRegistry::getRegistry();

    runner.addTest(registry.makeTest());
    try {
        runner.run(controller, testPath);

        // Print test in a compiler compatible format.
        CppUnit::CompilerOutputter outputter(&result, std::cerr);
        outputter.setLocationFormat("%f:%l: ");
        outputter.write();
    } catch (std::invalid_argument &e) {  // Test path not resolved
        std::cerr << std::endl << "ERROR: Test path not resolved: " << e.what() << std::endl;

        return 1;
    }

    return result.wasSuccessful() ? 0 : 1;
}
