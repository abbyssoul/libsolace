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
    SerialPortInfo() = default;

    SerialPortInfo(const Path& path, const String& desc, const String& hwId) :
        file(path), description(desc), hardwareId(hwId)
    {
        // No-op
    }

    /** Copy constructor */
    SerialPortInfo(const SerialPortInfo& other) = default;

    /** Move constructor */
    SerialPortInfo(SerialPortInfo&& other) = default;

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
    enum class Bytesize : uint8 {
        fivebits = 5,
        sixbits = 6,
        sevenbits = 7,
        eightbits = 8
    };

    /**
     * Parity types of the serial port
     */
    enum class Parity : uint8 {
        none = 0,
        odd = 1,
        even = 2,
        mark = 3,
        space = 4
    };

    /**
     * Stop bit types of the serial port
     */
    enum class Stopbits : uint8 {
        one = 1,
        two = 2,
        one_point_five
    };

    /**
     * Flowcontrol types of the serial port.
     */
    enum class Flowcontrol: uint8 {
        none = 0,
        software,
        hardware
    };


    using File::size_type;
    using File::read;
    using File::write;

public:

    /*! Destructor */
     ~Serial() override;

    // Disable copy constructors
    Serial(const Serial&) = delete;
    Serial& operator=(const Serial&) = delete;

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

};

}  // namespace IO
}  // namespace Solace
#endif  // SOLACE_IO_SERIAL_HPP
