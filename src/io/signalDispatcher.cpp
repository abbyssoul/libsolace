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
 * @file: io/signalDispatcher.cpp
 *
 *  Created by soultaker on 03/06/16.
*******************************************************************************/
#include "solace/io/signalDispatcher.hpp"
#include "solace/exception.hpp"


#include <map>

#include <unistd.h>
#include <signal.h>
#include <errno.h>


using namespace Solace;
using namespace Solace::IO;


std::multimap<int, std::function<void(int)> >&
getMapping() {
    static std::multimap<int, std::function<void(int)> > signalMap;
    return signalMap;
}


static void signalHandler(int signum) {
    auto& mapping = getMapping();

    auto ret = mapping.equal_range(signum);
    for (auto handlerRange = ret.first; handlerRange != ret.second; ++handlerRange) {
         handlerRange->second(signum);
    }
}


SignalDispatcher& SignalDispatcher::getInstance() {
    static SignalDispatcher processDispatcher;


    return processDispatcher;
}

SignalDispatcher::SignalDispatcher()
{

}

SignalDispatcher::~SignalDispatcher()
{

}


void SignalDispatcher::attachHandler(int signalNumber, const std::function<void(int)>& handler) {
    auto& mapping = getMapping();

    struct sigaction sigaction_info;
    sigaction_info.sa_handler = signalHandler;
    sigaction_info.sa_flags = 0;
    sigemptyset(&sigaction_info.sa_mask);

    struct sigaction old_action;
    if (sigaction(signalNumber, &sigaction_info, &old_action) < 0) {
        raise<IOException>(errno);
    }

    if (old_action.sa_handler && (old_action.sa_handler != signalHandler)) {
        mapping.emplace(signalNumber, [old_action](int signal) {
            if ((old_action.sa_flags & SA_SIGINFO) == SA_SIGINFO) {
                siginfo_t crap;
                old_action.sa_sigaction(signal, &crap, nullptr);
            } else {
                old_action.sa_handler(signal);
            }
        });
    }

    mapping.emplace(signalNumber, handler);
}
