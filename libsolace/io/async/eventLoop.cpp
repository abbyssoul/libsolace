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
 * @file: io/async/eventLoop.cpp
 *
 *  Created by soultaker on 01/10/16.
*******************************************************************************/
#include <solace/io/async/eventloop.hpp>

#include <algorithm>

#include <solace/io/file.hpp>

using namespace Solace;
using namespace Solace::IO;
using namespace Solace::IO::async;


Result::~Result() {

}


Result& EventLoop::submit(const std::shared_ptr<Request>& request) {
    _backlog.push_back(request);

    return request->promiss();
}


void noop(ISelectable*) {}

void EventLoop::run() {
    while (_keepOnRunning) {
        for (auto event : _selector.poll()) {

            std::shared_ptr<ISelectable> p(event.pollable, noop);

            if (std::find(_backlog.begin(), _backlog.end(), p) != _backlog.end()) {
//            if (_backlog.contains(event.pollable)) {
                auto request = static_cast<Request*>(event.pollable);

                if (event.events & Solace::IO::Selector::Events::Read) {
                    request->onRead();
                }

                if (event.events & Solace::IO::Selector::Events::Write) {
                    request->onWrite();
                }

                if (event.events & Solace::IO::Selector::Events::Error) {
                    request->onError();
                }
            }
        }
    }
}


void EventLoop::Request::onRead() {
    static_cast<File*>(_selectable)->read(_buffer);

    _future.resolve();
}

void EventLoop::Request::onWrite() {

}

void EventLoop::Request::onError() {

}
