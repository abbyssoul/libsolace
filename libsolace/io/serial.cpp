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
 * @file: solace/io/serial.cpp
 *
 *  Created by soultaker on 27/04/16.
*******************************************************************************/
#include <solace/io/serial.hpp>
#include <solace/exception.hpp>


#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

#if defined(__linux__)
# include <linux/serial.h>
#endif


using Solace::uint32;
using Solace::Path;
using Solace::ByteBuffer;
using Solace::IO::Serial;
using Solace::IO::IOException;



timespec timespec_from_ms(const uint32 millis) {
    timespec time;
    time.tv_sec = millis / 1e3;
    time.tv_nsec = (millis - (time.tv_sec * 1e3)) * 1e6;

    return time;
}


void reconfigurePort(int fd, uint32 baudrate, Serial::Bytesize bytesize,
                     Serial::Parity parity, Serial::Stopbits stopbits,
                     Serial::Flowcontrol flowcontrol)
{
    if (fd == -1) {
        // Can only operate on a valid file descriptor
        Solace::raise<IOException>("Invalid file descriptor, is the serial port open?");
    }

    termios options;
    if (tcgetattr(fd, &options) == -1) {
        Solace::raise<IOException>(errno, "tcgetattr");
    }

    // set up raw mode / no echo / binary
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG | IEXTEN);
    options.c_oflag &= ~(OPOST);
    options.c_iflag &= ~(INLCR | IGNCR | ICRNL | IGNBRK);
#ifdef IUCLC
    options.c_iflag &= ~IUCLC;
#endif
#ifdef PARMRK
    options.c_iflag &= ~PARMRK;
#endif

    // setup baud rate
    bool custom_baud = false;
    speed_t baud;
    switch (baudrate) {
#ifdef B0
        case 0: baud = B0; break;
#endif
#ifdef B50
        case 50: baud = B50; break;
#endif
#ifdef B75
        case 75: baud = B75; break;
#endif
#ifdef B110
        case 110: baud = B110; break;
#endif
#ifdef B134
        case 134: baud = B134; break;
#endif
#ifdef B150
        case 150: baud = B150; break;
#endif
#ifdef B200
        case 200: baud = B200; break;
#endif
#ifdef B300
        case 300: baud = B300; break;
#endif
#ifdef B600
        case 600: baud = B600; break;
#endif
#ifdef B1200
        case 1200: baud = B1200; break;
#endif
#ifdef B1800
        case 1800: baud = B1800; break;
#endif
#ifdef B2400
        case 2400: baud = B2400; break;
#endif
#ifdef B4800
        case 4800: baud = B4800; break;
#endif
#ifdef B7200
        case 7200: baud = B7200; break;
#endif
#ifdef B9600
        case 9600: baud = B9600; break;
#endif
#ifdef B14400
        case 14400: baud = B14400; break;
#endif
#ifdef B19200
        case 19200: baud = B19200; break;
#endif
#ifdef B28800
        case 28800: baud = B28800; break;
#endif
#ifdef B57600
        case 57600: baud = B57600; break;
#endif
#ifdef B76800
        case 76800: baud = B76800; break;
#endif
#ifdef B38400
        case 38400: baud = B38400; break;
#endif
#ifdef B115200
        case 115200: baud = B115200; break;
#endif
#ifdef B128000
        case 128000: baud = B128000; break;
#endif
#ifdef B153600
        case 153600: baud = B153600; break;
#endif
#ifdef B230400
        case 230400: baud = B230400; break;
#endif
#ifdef B256000
        case 256000: baud = B256000; break;
#endif
#ifdef B460800
        case 460800: baud = B460800; break;
#endif
#ifdef B576000
        case 576000: baud = B576000; break;
#endif
#ifdef B921600
        case 921600: baud = B921600; break;
#endif
#ifdef B1000000
        case 1000000: baud = B1000000; break;
#endif
#ifdef B1152000
        case 1152000: baud = B1152000; break;
#endif
#ifdef B1500000
        case 1500000: baud = B1500000; break;
#endif
#ifdef B2000000
        case 2000000: baud = B2000000; break;
#endif
#ifdef B2500000
        case 2500000: baud = B2500000; break;
#endif
#ifdef B3000000
        case 3000000: baud = B3000000; break;
#endif
#ifdef B3500000
        case 3500000: baud = B3500000; break;
#endif
#ifdef B4000000
        case 4000000: baud = B4000000; break;
#endif
        default:
            custom_baud = true;
            // OS X support
#if defined(MAC_OS_X_VERSION_10_4) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4)
        // Starting with Tiger, the IOSSIOSPEED ioctl can be used to set arbitrary baud rates
    // other than those specified by POSIX. The driver for the underlying serial hardware
    // ultimately determines which baud rates can be used. This ioctl sets both the input
    // and output speed.
    speed_t new_baud = static_cast<speed_t> (baudrate_);
    if (-1 == ioctl(fd, IOSSIOSPEED, &new_baud, 1)) {
      Solace::raise<IOException>(errno);
    }
    // Linux Support
#elif defined(__linux__) && defined(TIOCSSERIAL)
        struct serial_struct ser;

        if (-1 == ioctl(fd, TIOCGSERIAL, &ser)) {
            Solace::raise<IOException>(errno, "ioctl::TIOCGSERIAL");
        }

        // set custom divisor
        ser.custom_divisor = ser.baud_base / baudrate;

        // update flags
        ser.flags &= ~ASYNC_SPD_MASK;
        ser.flags |= ASYNC_SPD_CUST;

        if (-1 == ioctl(fd, TIOCSSERIAL, &ser)) {
            Solace::raise<IOException>(errno, "ioctl::TIOCSSERIAL");
        }
#else
        Solace::raise<IllegalArgumentException>("OS does not currently support custom bauds");
#endif
    }

    if (custom_baud == false) {
#ifdef _BSD_SOURCE
        if (-1 == ::cfsetspeed(&options, baud)) {
            Solace::raise<IOException>(errno, "cfsetspeed");
        }
#else
        if (-1 == ::cfsetispeed(&options, baud)) {
            Solace::raise<IOException>(errno, "cfsetispeed");
        }

        if (-1 == ::cfsetospeed(&options, baud)) {
            Solace::raise<IOException>(errno, "cfsetospeed");
        }
#endif
    }

    // setup char len
    options.c_cflag &= (tcflag_t) ~CSIZE;
    switch (bytesize) {
        case Serial::Bytesize::eightbits: options.c_cflag |= CS8; break;
        case Serial::Bytesize::sevenbits: options.c_cflag |= CS7; break;
        case Serial::Bytesize::sixbits:   options.c_cflag |= CS6; break;
        case Serial::Bytesize::fivebits:  options.c_cflag |= CS5; break;
    }

    // setup stopbits
    switch (stopbits) {
        case Serial::Stopbits::one: options.c_cflag &= ~(CSTOPB); break;
        case Serial::Stopbits::one_point_five:
        // ONE POINT FIVE same as TWO.. there is no POSIX support for 1.5
        options.c_cflag |=  (CSTOPB); break;
        case Serial::Stopbits::two: options.c_cflag |=  (CSTOPB); break;
    }

    // setup parity
    options.c_iflag &= ~(INPCK | ISTRIP);
    switch (parity) {
        case Serial::Parity::none: options.c_cflag &= ~(PARENB | PARODD); break;
        case Serial::Parity::even:
            options.c_cflag &= ~(PARODD);
            options.c_cflag |=  (PARENB);
            break;
        case Serial::Parity::odd:
            options.c_cflag |=  (PARENB | PARODD);
            break;
#ifdef CMSPAR
        case Serial::Parity::mark:
            options.c_cflag |=  (PARENB | CMSPAR | PARODD);
            break;
        case Serial::Parity::space:
            options.c_cflag |=  (PARENB | CMSPAR);
            options.c_cflag &= ~(PARODD);
            break;
#else
        // CMSPAR is not defined on OSX. So do not support mark or space parity.
    if (parity_ == parity_mark || parity_ == parity_space) {
        Solace::raise<IllegalArgumentException>("OS does not support mark or space parity");
    }
#endif  // ifdef CMSPAR
    }

    bool xonxoff_ = false;
    bool rtscts_ = false;
    // setup flow control
    switch (flowcontrol) {
        case Serial::Flowcontrol::none:
            xonxoff_ = false;
            rtscts_ = false;
            break;
        case Serial::Flowcontrol::software:
            xonxoff_ = true;
            rtscts_ = false;
            break;
        case Serial::Flowcontrol::hardware:
            xonxoff_ = false;
            rtscts_ = true;
            break;
    }

    // xonxoff
#ifdef IXANY
    if (xonxoff_)
        options.c_iflag |=  (IXON | IXOFF);
    else
        options.c_iflag &= ~(IXON | IXOFF | IXANY);
#else
    if (xonxoff_)
        options.c_iflag |=  (IXON | IXOFF);
    else
        options.c_iflag &= ~(IXON | IXOFF);
#endif
    // rtscts
#ifdef CRTSCTS
    if (rtscts_)
        options.c_cflag |=  (CRTSCTS);
    else
        options.c_cflag &= ~(CRTSCTS);
#elif defined CNEW_RTSCTS
    if (rtscts_)
        options.c_cflag |=  (CNEW_RTSCTS);
    else
        options.c_cflag &= ~(CNEW_RTSCTS);
#else
#warning "OS Support seems wrong."
#endif

    // http://www.unixwiz.net/techtips/termios-vmin-vtime.html
    // Sets read to be a polling read, so one is better to use select to ensure there is data available
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 0;

    // activate settings
    if (::tcsetattr(fd, TCSANOW, &options) != 0) {
        Solace::raise<IOException>(errno, "tcsetattr(TCSANOW)");
    }
}


Serial::Serial(const Path& file, uint32 baudrate,
               Bytesize bytesize, Parity parity, Stopbits stopbits,
               Flowcontrol flowcontrol) : File(file, O_RDWR | O_NOCTTY | O_NONBLOCK)
{
    const auto fd = validateFd();
    reconfigurePort(fd, baudrate, bytesize, parity, stopbits, flowcontrol);
}


Serial::Serial(Serial&& other) : File(std::move(other)) {
    // No-op
}


Serial::~Serial() {
    // No-op
}

Serial& Serial::swap(Serial& rhs) noexcept {
    File::swap(rhs);

    return *this;
}


void Serial::flush() {
    const auto fd = validateFd();

    tcdrain(fd);
}


void Serial::flushInput() {
    const auto fd = validateFd();

    tcflush(fd, TCIFLUSH);
}


void Serial::flushOutput() {
    const auto fd = validateFd();

    tcflush(fd, TCOFLUSH);
}


void Serial::sendBreak(int duration) {
    const auto fd = validateFd();

    tcsendbreak(fd, static_cast<int>(duration / 4));
}


void Serial::setBreak(bool level) {
    const auto fd = validateFd();

    if (level) {
        if (-1 == ioctl(fd, TIOCSBRK)) {
            Solace::raise<IOException>(errno, "ioctl(fd, TIOCSBRK)");
        }
    } else {
        if (-1 == ioctl(fd, TIOCCBRK)) {
            Solace::raise<IOException>(errno, "ioctl(fd, TIOCCBRK)");
        }
    }
}


void Serial::setRTS(bool level) {
    const auto fd = validateFd();

    int command = TIOCM_RTS;
    if (level) {
        if (-1 == ioctl(fd, TIOCMBIS, &command)) {
            Solace::raise<IOException>(errno, "ioctl(TIOCMBIS)");
        }
    } else {
        if (-1 == ioctl(fd, TIOCMBIC, &command)) {
            Solace::raise<IOException>(errno, "ioctl(TIOCMBIC)");
        }
    }
}

void Serial::setDTR(bool level) {
    const auto fd = validateFd();

    int command = TIOCM_DTR;
    if (level) {
        if (-1 == ioctl(fd, TIOCMBIS, &command)) {
            Solace::raise<IOException>(errno, "ioctl(TIOCMBIS)");
        }
    } else {
        if (-1 == ioctl(fd, TIOCMBIC, &command)) {
            Solace::raise<IOException>(errno, "ioctl(TIOCMBIC)");
        }
    }
}

bool Serial::getCTS() {
    const auto fd = validateFd();

    int status;
    if (-1 == ioctl(fd, TIOCMGET, &status)) {
        Solace::raise<IOException>(errno, "ioctl(TIOCMGET)");
    }

    return 0 != (status & TIOCM_CTS);
}


bool Serial::getDSR() {
    const auto fd = validateFd();

    int status;
    if (-1 == ioctl(fd, TIOCMGET, &status)) {
        Solace::raise<IOException>(errno, "ioctl(TIOCMGET)");
    }

    return 0 != (status & TIOCM_DSR);
}


bool Serial::getRI() {
    const auto fd = validateFd();

    int status;
    if (-1 == ioctl(fd, TIOCMGET, &status)) {
        Solace::raise<IOException>(errno, "ioctl(TIOCMGET)");
    }

    return 0 != (status & TIOCM_RI);
}


bool Serial::getCD() {
    const auto fd = validateFd();

    int status;
    if (-1 == ioctl(fd, TIOCMGET, &status)) {
        Solace::raise<IOException>(errno);
    }

    return 0 != (status & TIOCM_CD);
}


bool Serial::waitForChange() {
    const auto fd = validateFd();

#ifndef TIOCMIWAIT
    while (isOpened()) {
        int status;

        if (-1 == ioctl(fd, TIOCMGET, &status)) {
            Solace::raise<IOException>(errno);
        } else {
            if (0 != (status & TIOCM_CTS)
             || 0 != (status & TIOCM_DSR)
             || 0 != (status & TIOCM_RI)
             || 0 != (status & TIOCM_CD))
            {
              return true;
            }
        }

        usleep(1000);
    }

    return false;
#else
  int command = (TIOCM_CD | TIOCM_DSR | TIOCM_RI | TIOCM_CTS);

  if (-1 == ioctl(fd, TIOCMIWAIT, &command)) {
      Solace::raise<IOException>(errno, "ioctl(TIOCMIWAIT)");
  }

  return true;
#endif
}

bool Serial::waitReadable(uint32 timeout) {
    const auto fd = validateFd();

    // Setup a select call to block for serial data or a timeout
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    timespec timeout_ts(timespec_from_ms(timeout));
    const int r = pselect(fd + 1, &readfds, nullptr, nullptr, &timeout_ts, nullptr);

    if (r < 0) {
        // Select was interrupted
        if (errno == EINTR) {
            return false;
        }

        // Otherwise there was some error
        Solace::raise<IOException>(errno, "pselect");
    }

    // Timeout occurred
    if (r == 0) {
        return false;
    }

    // This shouldn't happen, if r > 0 our fd has to be in the list!
    if (!FD_ISSET(fd, &readfds)) {
        Solace::raise<IOException>("pselect reported ready, but our fd isn't in the list, Not suppose to happen!");
    }

    // Data available to read.
    return true;
}


Serial::size_type Serial::available() const {
    const auto fd = validateFd();

    int count = 0;
    if (-1 == ioctl(fd, TIOCINQ, &count)) {
        Solace::raise<IOException>(errno, "ioctl(TIOCINQ)");
    }

    return static_cast<size_type>(count);
}

