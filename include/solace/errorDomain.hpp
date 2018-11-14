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
 * libSolace: Domain for error codes
 *	@file		solace/errorDomain.hpp
 ******************************************************************************/
#pragma once
#ifndef SOLACE_ERRORDOMAIN_HPP
#define SOLACE_ERRORDOMAIN_HPP

#include "solace/types.hpp"
#include "solace/atom.hpp"
#include "solace/optional.hpp"
#include "solace/stringView.hpp"


namespace Solace {


/**
 * Base class for error domains.
 */
class ErrorDomain {
public:
    virtual ~ErrorDomain() noexcept = default;

    ErrorDomain() = default;
    ErrorDomain(ErrorDomain const&) = delete;
    ErrorDomain(ErrorDomain&&) = default;
    ErrorDomain& operator= (ErrorDomain const&) = delete;
    ErrorDomain& operator= (ErrorDomain&&) = default;

    virtual StringView getName() const noexcept = 0;
    virtual StringView getMessage(int code) const noexcept = 0;
};



/**
 * Lookup error domain/category by atom value
 * @param categoryId Atom identifing error domain
 * @return An error domain if one was found.
 */
Optional<ErrorDomain*> getErrorDomain(AtomValue categoryId) noexcept;


extern const AtomValue kDefaultCatergory;
extern const AtomValue kSystemCatergory;


enum class BasicError : int {
    Overflow = 0,
    InvalidInput,
};

enum class AsyncError: int {
    AsyncError
};

enum class GenericError : int {
    PERM = 1,       /* Operation not permitted */
    NOENT = 2,      /* No such file or directory */
    SRCH = 3,       /* No such process */
    INTR = 4,       /* Interrupted system call */
    IO = 5,         /* I/O error */
    NXIO = 6,       /* No such device or address */
    TooBig = 7,     /* Argument list too long */
    NOEXEC = 8,     /* Exec format error */
    BADF = 9,       /* Bad file number */
    CHILD = 10,     /* No child processes */
    AGAIN = 11,     /* Try again */
    NOMEM = 12,     /* Out of memory */
    ACCES = 13,     /* Permission denied */
    FAULT = 14,     /* Bad address */
    NOTBLK = 15,    /* Block device required */
    BUSY = 16,      /* Device or resource busy */
    EXIST = 17,     /* File exists */
    XDEV = 18,      /* Cross-device link */
    NODEV = 19,     /* No such device */
    NOTDIR = 20,    /* Not a directory */
    ISDIR = 21,     /* Is a directory */
    INVAL = 22,     /* Invalid argument */
    NFILE = 23,     /* File table overflow */
    MFILE = 24,     /* Too many open files */
    NOTTY = 25,     /* Not a typewriter */
    TXTBSY = 26,    /* Text file busy */
    FBIG = 27,      /* File too large */
    NOSPC = 28,     /* No space left on device */
    SPIPE = 29,     /* Illegal seek */
    ROFS = 30,      /* Read-only file system */
    MLINK = 31,     /* Too many links */
    PIPE = 32,      /* Broken pipe */
    DOM = 33,       /* Math argument out of domain of func */
    RANGE = 34,     /* Math result not representable */
};


enum class SystemErrors : int {
    DEADLOCK = 35,      /* Resource deadlock would occur */
    NameTooLong = 36,   /* File name too long */
    NoLock = 37,        /* No record locks available */
    NoSys = 38,         /* Invalid system call number */

    NOTEMPTY = 39,      /* Directory not empty */
    LOOP = 40,          /* Too many symbolic links encountered */
    WOULDBLOCK = 11,    /* Operation would block */
    NOMSG = 42,         /* No message of desired type */
    IDRM = 43,          /* Identifier removed */
    CHRNG = 44,         /* Channel number out of range */
    L2NSYNC = 45,       /* Level 2 not synchronized */
    L3HLT = 46,         /* Level 3 halted */
    L3RST = 47,         /* Level 3 reset */
    LNRNG = 48,         /* Link number out of range */
    UNATCH = 49,        /* Protocol driver not attached */
    NOCSI = 50,         /* No CSI structure available */
    L2HLT = 51,         /* Level 2 halted */
    BADE = 52,          /* Invalid exchange */
    BADR = 53,          /* Invalid request descriptor */
    XFULL = 54,         /* Exchange full */
    NOANO = 55,         /* No anode */
    BADRQC = 56,        /* Invalid request code */
    BADSLT = 57,        /* Invalid slot */
    BFONT = 59,         /* Bad font file format */
    NOSTR = 60,         /* Device not a stream */
    NODATA = 61,        /* No data available */
    TIME = 62,          /* Timer expired */
    NOSR = 63,          /* Out of streams resources */
    NONET = 64,         /* Machine is not on the network */
    NOPKG = 65,         /* Package not installed */
    REMOTE = 66,        /* Object is remote */
    NOLINK = 67,        /* Link has been severed */
    ADV = 68,           /* Advertise error */
    SRMNT = 69,         /* Srmount error */
    COMM = 70,          /* Communication error on send */
    PROTO = 71,         /* Protocol error */
    MULTIHOP = 72,      /* Multihop attempted */
    DOTDOT = 73,        /* RFS specific error */
    BADMSG = 74,        /* Not a data message */
    Overflow = 75,      /* Value too large for defined data type */
    NOTUNIQ = 76,       /* Name not unique on network */
    BADFD = 77,         /* File descriptor in bad state */
    REMCHG = 78,        /* Remote address changed */
    LIBACC = 79,        /* Can not access a needed shared library */
    LIBBAD = 80,        /* Accessing a corrupted shared library */
    LIBSCN = 81,        /* .lib section in a.out corrupted */
    LIBMAX = 82,        /* Attempting to link in too many shared libraries */
    LIBEXEC = 83,       /* Cannot exec a shared library directly */
    ILSEQ = 84,         /* Illegal byte sequence */
    RESTART = 85,       /* Interrupted system call should be restarted */
    STRPIPE = 86,       /* Streams pipe error */
    USERS = 87,         /* Too many users */
    NOTSOCK = 88,       /* Socket operation on non-socket */
    DESTADDRREQ = 89,   /* Destination address required */
    MSGSIZE = 90,       /* Message too long */
    PROTOTYPE = 91,     /* Protocol wrong type for socket */
    NOPROTOOPT = 92,    /* Protocol not available */
    PROTONOSUPPORT = 93,    /* Protocol not supported */
    SOCKTNOSUPPORT = 94,    /* Socket type not supported */
    OPNOTSUPP = 95,     /* Operation not supported on transport endpoint */
    PFNOSUPPORT = 96,   /* Protocol family not supported */
    AFNOSUPPORT = 97,   /* Address family not supported by protocol */
    ADDRINUSE = 98,     /* Address already in use */
    ADDRNOTAVAIL = 99,  /* Cannot assign requested address */
    NETDOWN = 100,      /* Network is down */
    NETUNREACH = 101,   /* Network is unreachable */
    NETRESET = 102,     /* Network dropped connection because of reset */
    CONNABORTED = 103,  /* Software caused connection abort */
    CONNRESET = 104,    /* Connection reset by peer */
    NOBUFS = 105,       /* No buffer space available */
    ISCONN = 106,       /* Transport endpoint is already connected */
    NOTCONN = 107,      /* Transport endpoint is not connected */
    SHUTDOWN = 108,     /* Cannot send after transport endpoint shutdown */
    TOOMANYREFS = 109,  /* Too many references: cannot splice */
    TIMEDOUT = 110,     /* Connection timed out */
    CONNREFUSED = 111,  /* Connection refused */
    HOSTDOWN = 112,     /* Host is down */
    HOSTUNREACH = 113,  /* No route to host */
    ALREADY = 114,      /* Operation already in progress */
    INPROGRESS = 115,   /* Operation now in progress */
    STALE = 116,        /* Stale file handle */
    UCLEAN = 117,       /* Structure needs cleaning */
    NOTNAM = 118,       /* Not a XENIX named type file */
    NAVAIL = 119,       /* No XENIX semaphores available */
    ISNAM = 120,        /* Is a named type file */
    REMOTEIO = 121,     /* Remote I/O error */
    DQUOT = 122,        /* Quota exceeded */

    NOMEDIUM = 123,     /* No medium found */
    MEDIUMTYPE = 124,   /* Wrong medium type */
    CANCELED = 125,     /* Operation Canceled */
    NOKEY = 126,        /* Required key not available */
    KEYEXPIRED = 127,   /* Key has expired */
    KEYREVOKED = 128,   /* Key has been revoked */
    KEYREJECTED = 129,  /* Key was rejected by service */

    /* for robust mutexes */
    OWNERDEAD = 130,        /* Owner died */
    NOTRECOVERABLE = 131,   /* State not recoverable */

    RFKILL = 132,           /* Operation not possible due to RF-kill */
    HWPOISON = 133,         /* Memory page has hardware error */
};



}  // End of namespace Solace
#endif  // SOLACE_ERRORDOMAIN_HPP
