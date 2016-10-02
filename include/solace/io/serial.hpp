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
 * libSolace: Serial Port object
 *	@file		solace/io/selectable.hpp
 *	@author		$LastChangedBy$
 *	@date		Created on 26/04/16
 *	ID:			$Id$
 ******************************************************************************/
#pragma once
#ifndef SOLACE_IO_SERIAL_HPP
#define SOLACE_IO_SERIAL_HPP


#include "solace/io/file.hpp"


namespace Solace { namespace IO {

/**
 * Serial device descriptor
 */
struct SerialPortInfo {

    /*! File name of the serial port (this can be passed to the constructor of Serial). */
    Path file;

    /*! Human readable description of serial device if available. */
    String description;

    /*! Hardware ID (e.g. VID:PID of USB serial devices) or "" (empty string) if not available. */
    String hardwareId;


    /** Default Constructor for collection interop */
    SerialPortInfo() {}

    SerialPortInfo(const Path& path, const String& desc, const String& hwId) :
        file(path), description(desc), hardwareId(hwId)
    {
        // No-op
    }

    /** Copy constructor */
    SerialPortInfo(const SerialPortInfo& other) :
        file(other.file),
        description(other.description),
        hardwareId(other.hardwareId)
    {
        // No-op
    }

    /** Move constructor */
    SerialPortInfo(SerialPortInfo&& other) :
        file(std::move(other.file)),
        description(std::move(other.description)),
        hardwareId(std::move(other.hardwareId))
    {
        // No-op
    }

    bool operator== (const SerialPortInfo& rhs) const {
        return (file == rhs.file &&
                description == rhs.description &&
                hardwareId == hardwareId);
    }

    SerialPortInfo& swap(SerialPortInfo& rhs) {
        file.swap(rhs.file);
        description.swap(rhs.description);
        hardwareId.swap(rhs.hardwareId);

        return *this;
    }

    SerialPortInfo& operator= (SerialPortInfo&& rhs) noexcept {
        return swap(rhs);
    }
};


/**
 * Serial port device file
 * 
 */
class Serial :
        public File {
public:

    /**
     * Enumerate serial ports currently available
     * @return A collection of serial port descriptors
     */
    static Array<SerialPortInfo> enumeratePorts();

public:

    /**
     * Byte sizes of the serial port
     */
    enum class Bytesize: int {
        fivebits = 5,
        sixbits = 6,
        sevenbits = 7,
        eightbits = 8
    };

    /**
     * Parity types of the serial port
     */
    enum class Parity: int {
        none = 0,
        odd = 1,
        even = 2,
        mark = 3,
        space = 4
    };

    /**
     * Stop bit types of the serial port
     */
    enum class Stopbits: int {
        one = 1,
        two = 2,
        one_point_five
    };

    /**
     * Flowcontrol types of the serial port.
     */
    enum class Flowcontrol: int {
        none = 0,
        software,
        hardware
    };


    using File::size_type;
    using File::read;
    using File::write;

public:
    /**
    * Open the port and create a Serial object incapsulating it.
    *
    * \param port A Path to the serial device file 
    *            Something like '/dev/ttyS0' on Linux.
    *
    * \param baudrate An baud rate
    *
    * \param timeout A serial::Timeout struct that defines the timeout
    * conditions for the serial port. \see serial::Timeout
    *
    * \param bytesize Size of each byte in the serial transmission of data,
    * default is eightbits, possible values are: fivebits, sixbits, sevenbits,
    * eightbits
    *
    * \param parity Method of parity, default is parity_none, possible values
    * are: parity_none, parity_odd, parity_even
    *
    * \param stopbits Number of stop bits used, default is stopbits_one,
    * possible values are: stopbits_one, stopbits_one_point_five, stopbits_two
    *
    * \param flowcontrol Type of flowcontrol used
    *
    * \throw serial::IOException
    * \throw std::invalid_argument
    */
    Serial(const Path& file,
        uint32 baudrate = 9600,
        Bytesize bytesize = Bytesize::eightbits,
        Parity parity = Parity::none,
        Stopbits stopbits = Stopbits::one,
        Flowcontrol flowcontrol = Flowcontrol::none);

    /**
     * Move constructor
     * \param other serial port to move representation from
     */
    Serial(Serial&& other);


    /*! Destructor */
    virtual ~Serial();


    /**
     * Swap content of this file with an other
     *
     * \param rhs A serial port file handle to swap with
     *
     * \return Reference to this
     */
    Serial& swap(Serial& rhs) noexcept;


    /**
     * Move assignment operator
     *
     * \param rhs Other file to move data from
     *
     * \return reference to this
     */
    Serial& operator= (Serial&& rhs) noexcept {
        return swap(rhs);
    }


    /** Read a given number of bytes from the serial port into a the buffer.
    *
    * The read function will return in one of three cases:
    *  * The number of requested bytes was read.
    *    * In this case the number of bytes requested will match the size_t
    *      returned by read.
    *  * A timeout occurred, in this case the number of bytes read will not
    *    match the amount requested, but no exception will be thrown.  One of
    *    two possible timeouts occurred:
    *    * The inter byte timeout expired, this means that number of
    *      milliseconds elapsed between receiving bytes from the serial port
    *      exceeded the inter byte timeout.
    *    * The total timeout expired, which is calculated by multiplying the
    *      read timeout multiplier by the number of requested bytes and then
    *      added to the read timeout constant.  If that total number of
    *      milliseconds elapses after the initial call to read a timeout will
    *      occur.
    *  * An exception occurred, in this case an actual exception will be thrown.
    *
    * \param buffer A write buffer to write date into
    * \param bytesToRead Number of bytes to be read.
    *
    * \return The number of bytes actually read.
    *
    * \throw serial::SerialException
    */
//    size_type read(ByteBuffer& buffer, ByteBuffer::size_type bytesToRead) override;


    /*! Write given number of bytes into the serial port.
    *
    * \param data The data buffer to be written to the serial port.
    * \param bytesToWrite How many bytes should be written from the buffer.
    *
    * \return The number of bytes actually written to the serial port.
    *
    * \throw serial::SerialException
    * \throw serial::IOException
    */
//    size_type write(MemoryView& data, MemoryView::size_type bytesToWrite) override;

    /**
     * \see File:flush
     */
    void flush() override;


    /*! Flush only the input buffer */
    void flushInput();

    /*! Flush only the output buffer */
    void flushOutput();

    /*! Sends the RS-232 break signal.
    * See tcsendbreak(3) for more details.
    */
    void sendBreak(int duration);

    /*! Set the break condition to a given level.
    * Defaults to true.
    */
    void setBreak(bool level = true);

    /*! Set the RTS handshaking line to the given level.
    *  Defaults to true.
    */
    void setRTS(bool level = true);

    /*! Set the DTR handshaking line to the given level.
    */
    void setDTR(bool level = true);

    /*! Returns the current status of the CTS line. */
    bool getCTS();

    /*! Returns the current status of the DSR line. */
    bool getDSR();

    /*! Returns the current status of the RI line. */
    bool getRI();

    /*! Returns the current status of the CD line. */
    bool getCD();

    /*! Return the number of byte available for read */
    size_type available() const;

    /*!
    * Blocks until CTS, DSR, RI, CD changes or something interrupts it.
    *
    * Can throw an exception if an error occurs while waiting.
    * You can check the status of CTS, DSR, RI, and CD once this returns.
    * Uses TIOCMIWAIT via ioctl if available (mostly only on Linux) with a
    * resolution of less than +-1ms and as good as +-0.2ms.  Otherwise a
    * polling method is used which can give +-2ms.
    *
    * \return Returns true if one of the lines changed, false if something else
    * occurred.
    *
    * \throw SerialException
    */
    bool waitForChange();

    /*! Block until there is serial data to read or read_timeout_constant
    * number of milliseconds have elapsed. The return value is true when
    * the function exits with the port in a readable state, false otherwise
    * (due to timeout or select interruption). */
    bool waitReadable(uint32 timeout);

private:

    // Disable copy constructors
    Serial(const Serial&) = delete;
    Serial& operator=(const Serial&) = delete;

};

}  // namespace IO
}  // namespace Solace
#endif  // SOLACE_IO_SERIAL_HPP
