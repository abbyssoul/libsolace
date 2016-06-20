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
 * libSolace: Unix File handle wrapper
 *	@file		solace/io/file.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_SELECTOR_HPP
#define SOLACE_IO_SELECTOR_HPP

#include "solace/io/selectable.hpp"
#include "solace/io/ioexception.hpp"

#include <memory>  // std::shared_ptr<>


namespace Solace { namespace IO {

/**
 * Base class for the system specific poll mechanism
 */
class Selector {
public:
    class IPollerImpl;


    static Selector epoll(uint eventSize);


public:

    enum Events {
        Read = 0x001,
        Write = 0x004,
        Error = 0x008,
        Hup = 0x010,
    };

    struct Event {
        int             events;
        ISelectable*    pollable;
        void*           data;
    };

    class Iterator {
    public:
        bool operator!= (const Iterator& other) const {
            return ((_index != other._index) ||
                    (_size != other._size) ||
                    (_pimpl.get() != other._pimpl.get()));
        }

        bool operator== (const Iterator& other) const {
            return ((_index == other._index) &&
                    (_pimpl.get() == other._pimpl.get()));
        }

        const Iterator& operator++ ();

        Event operator* () const {
            return this->operator ->();
        }

        Event operator-> () const;

        Iterator begin() const {
            return Iterator(_size, _pimpl);
        }

        Iterator end() const {
            return Iterator(_size, _size, _pimpl);
        }

        Iterator& swap(Iterator& rhs) noexcept {
            std::swap(_index, rhs._index);
            std::swap(_size, rhs._size);
            std::swap(_pimpl, rhs._pimpl);

            return *this;
        }

        uint getSize() const noexcept {
            return _size;
        }

        Iterator(uint index, uint size, const std::shared_ptr<IPollerImpl>& p): _index(index), _size(size),
            _pimpl(p)
        {}

        Iterator(uint size, const std::shared_ptr<IPollerImpl>& p): _index(0), _size(size),
            _pimpl(p)
        {}

        Iterator(Iterator&& rhs): _index(rhs._index), _size(rhs._size),
            _pimpl(std::move(rhs._pimpl))
        {}

        Iterator& operator= (Iterator&& rhs) noexcept  {
            return swap(rhs);
        }

    private:
        uint _index;
        uint _size;

        std::shared_ptr<IPollerImpl> _pimpl;
    };

public:

    virtual ~Selector() = default;

    //--------------------------------------------------------------------------
    // Pollable object management:
    //--------------------------------------------------------------------------
    /**
     * Add new pollable object to be polled
     * @param selectable - a pollable object to register for polling.
     * @param events - Or'd mask of events to listen for. @see Events
     *
     * TODO(abbyssoul): should return Result<>
     */
    void add(ISelectable* selectable, int events);

    /**
     * Deregister the pollable object
     * @param selectable - a pollable object to deregister
     *
     */
    void remove(const ISelectable* selectable);

    /**
     * Wait for events on the previously added selectable items.
     *
     * @param msec - The maximum wait time in milliseconds (-1 == infinite) or 0 to return immidiately.
     * @return Iterator to
     */
    Iterator poll(uint msec = -1);

private:
    std::shared_ptr<IPollerImpl> _pimpl;

    Selector(const std::shared_ptr<IPollerImpl>& impl);
    Selector(std::shared_ptr<IPollerImpl>&& impl);
};


}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_SELECTOR_HPP
