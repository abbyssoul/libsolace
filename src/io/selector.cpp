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
 * @file: io/selector.cpp
 *
 *  Created by soultaker on 27/04/16.
*******************************************************************************/
#include <solace/io/selector.hpp>
#include <solace/exception.hpp>
#include <solace/array.hpp>

#include "selector_impl.hpp"


using namespace Solace;
using namespace Solace::IO;



Selector::Iterator Selector::Iterator::begin() const {
    return Iterator(_pimpl, _index, _size);
}

Selector::Iterator Selector::Iterator::end() const {
    return Iterator(_pimpl, _size, _size);
}

Selector::Iterator const&
Selector::Iterator::operator++ () {
    assertIndexInRange(_index, 0, _size, "iterator");

    _index = _pimpl->advance(_index);

    return (*this);
}


Selector::Event Selector::Iterator::operator-> () const {
    return _pimpl->getEvent(_index);
}


Selector::~Selector() = default;

Selector::Selector(Selector&& rhs) noexcept = default;

Selector::Selector(std::unique_ptr<IPollerImpl>&& impl): _pimpl(std::move(impl)) {
}

void Selector::add(ISelectable* selectable, int events) {
    _pimpl->add(selectable, events);
}

void Selector::add(ISelectable::poll_id fd, int events, void* data) {
    _pimpl->add(fd, events, data);
}

void Selector::addRaw(ISelectable::poll_id fd, int events, void* data) {
    _pimpl->addRaw(fd, events, data);
}


void Selector::remove(const ISelectable* selectable) {
    _pimpl->remove(selectable);
}


void Selector::remove(ISelectable::poll_id fd) {
    _pimpl->remove(fd);
}


Selector::Iterator Selector::poll(int msec) {
    auto r = _pimpl->poll(msec);

    return Selector::Iterator(_pimpl.get(), std::get<0>(r), std::get<1>(r));
}
