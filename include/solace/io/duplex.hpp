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
 * libSolace: Full duplex file channel
 *	@file		solace/io/duplex.hpp
 *	@author		$LastChangedBy$
 *	@date		$LastChangedDate$
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_DUPLEX
#define SOLACE_IO_DUPLEX

#include "solace/io/file.hpp"


namespace Solace { namespace IO {

/**
 * Duplex is a simple file based multiplexor
 * Duplex provides full read write file interface agregating two file descriptors.
 */
class Duplex :
        public IOObject {
public:
    typedef ISelectable::poll_id poll_id;

    using IOObject::read;
    using IOObject::write;

public:

    ~Duplex();

    Duplex(const Duplex& rhs) = delete;
    Duplex& operator= (const Duplex& rhs) = delete;

    /**
     * Initialize a duplix aggregating two opened file descriptors
     * @param inFid File descriptor opened for read
     * @param outFid File descriptor opened for write
     */
    Duplex(poll_id inFid, poll_id outFid) :
        Duplex(File::fromFd(inFid), File::fromFd(outFid))
    {}

    Duplex(File && inFile, File && outFile) :
        _in(std::move(inFile)),
        _out(std::move(outFile))
    {}

    Duplex(Duplex&& rhs) :
        _in(std::move(rhs._in)),
        _out(std::move(rhs._out))
    {}


    /**
     * Move assignment operator
     * @param rhs Other file to move data from
     * @return reference to this
     */
    Duplex& operator= (Duplex&& rhs) noexcept {
        return swap(rhs);
    }

    /**
     * Swap content of this file with an other
     * @param rhs A file handle to swap with
     * @return Reference to this
     */
    Duplex& swap(Duplex& rhs) noexcept {
        using std::swap;

        swap(_in, rhs._in);
        swap(_out, rhs._out);

        return *this;
    }


    // IOObject read/write interface
    IOObject::IOResult read(MemoryView& buffer) override {
        return _in.read(buffer);
    }

    IOObject::IOResult write(const ImmutableMemoryView& buffer) override {
        return _out.write(buffer);
    }

    bool isOpened() const override {
        return (_in.isOpened() || _out.isOpened());
    }

    void close() override;

    virtual void flush();

    File& getReadEnd() noexcept {
        return _in;
    }

    const File& getReadEnd() const noexcept {
        return _in;
    }

    File& getWriteEnd() {
        return _out;
    }

    const File& getWriteEnd() const noexcept {
        return _out;
    }

private:
    File _in;
    File _out;
};


inline void swap(Duplex& lhs, Duplex& rhs) noexcept {
    lhs.swap(rhs);
}

}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_DUPLEX
