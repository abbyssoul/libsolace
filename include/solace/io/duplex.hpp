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
class Duplex: public File {
public:
    using File::poll_id;

public:

    /**
     * Initialize a duplix aggregating two opened file descriptors
     * @param inFid File descriptor opened for read
     * @param outFid File descriptor opened for write
     */
    Duplex(poll_id inFid, poll_id outFid): Duplex(File::fromFd(inFid), File::fromFd(outFid)) {

    }

    Duplex(File && inFile, File && outFile) :
        _in(std::move(inFile)),
        _out(std::move(outFile))
    {}

    Duplex(Duplex&& rhs) :
        _in(std::move(rhs._in)),
        _out(std::move(rhs._out))
    {}

    ~Duplex();

    // File read/write interface
    size_type read(MemoryView& buffer, MemoryView::size_type bytesToRead) override {
        return _in.read(buffer, bytesToRead);
    }

    size_type write(const MemoryView& buffer, MemoryView::size_type bytesToWrite) override {
        return _out.write(buffer, bytesToWrite);
    }

//    size_type seek(size_type offset, Seek type) override;
    void close() override;
    void flush() override;


    File& getReadEnd() {
        return _in;
    }

    const File& getReadEnd() const {
        return _in;
    }

    File& getWriteEnd() {
        return _out;
    }

    const File& getWriteEnd() const {
        return _out;
    }

private:
    File _in;
    File _out;
};

}  // End of namespace IO
}  // End of namespace Solace
#endif  // SOLACE_IO_DUPLEX
