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
 *	@file test/mockTypes.cpp
 ******************************************************************************/

#include "mockTypes.hpp"

int PimitiveType::InstanceCount = 0;
int SimpleType::InstanceCount = 0;
int MoveOnlyType::InstanceCount = 0;
int SometimesConstructable::InstanceCount = 0;

int SometimesConstructable::BlowUpEveryInstance = 4;


std::ostream& operator<< (std::ostream& ostr, const SimpleType& t) {
    return ostr << "SimpleType(" << t.x << ", " << t.y << ", " << t.z << ")";
}
