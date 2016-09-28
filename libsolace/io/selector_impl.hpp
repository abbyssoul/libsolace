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
 * @file: io/selector_impl.hpp
 *
 *  Created by soultaker on 20/09/16.
*******************************************************************************/
#pragma once
#ifndef SELECTOR_IMPL_H
#define SELECTOR_IMPL_H


#include <tuple>


namespace Solace {


class IO::Selector::IPollerImpl {
public:

    virtual ~IPollerImpl() = default;

    virtual void add(ISelectable* selectable, int events) = 0;

    virtual void remove(const ISelectable* selectable) = 0;

    virtual std::tuple<uint, uint> poll(uint32 msec) = 0;

    virtual Selector::Event getEvent(uint i) = 0;

    virtual int advance(uint i) = 0;
};


}  // End of namespace Solace
#endif  // SELECTOR_IMPL_H
