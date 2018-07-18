/*
*  Copyright 2018 Ivan Ryabov
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
 * libsolace Unit Test Suit
 * @file: GTest entry point
 *******************************************************************************/
#include <gtest/gtest.h>
#include "ci/teamcity_gtest.h"

int main(int argc, char **argv) {

    testing::InitGoogleTest(&argc, argv);

    // to do: Figure out what to do with this.
    if (jetbrains::teamcity::underTeamcity()) {
        auto& listeners = testing::UnitTest::GetInstance()->listeners();

        // Add unique flowId parameter if you want to run test processes in parallel
        // See http://confluence.jetbrains.net/display/TCD6/Build+Script+Interaction+with+TeamCity#BuildScriptInteractionwithTeamCity-MessageFlowId
        listeners.Append(new jetbrains::teamcity::TeamcityGoogleTestEventListener());
    }

    return RUN_ALL_TESTS();
}
