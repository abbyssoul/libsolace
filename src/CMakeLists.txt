
set(SOURCE_FILES
        assert.cpp
        exception.cpp
        errorDomain.cpp
        error.cpp
        atom.cpp
        char.cpp

        memoryView.cpp
        mutableMemoryView.cpp
        memoryResource.cpp
        memoryManager.cpp
        byteReader.cpp
        byteWriter.cpp

        array.cpp
        base16.cpp
#        base32.cpp
        base64.cpp
        parseUtils.cpp
        string.cpp
        stringBuilder.cpp
        stringView.cpp

        version.cpp
        path.cpp
        encoder.cpp
        env.cpp
        uuid.cpp

        ioobject.cpp
        io/file.cpp
        io/duplex.cpp
        io/pipe.cpp
        io/serial.cpp
        io/serial_list_ports.cpp
        io/signalDispatcher.cpp
        io/selector.cpp
        io/selector_poll.cpp
        io/selector_epoll.cpp
        io/platformFilesystem.cpp
        io/sharedMemory.cpp

        cli/helpPrinter.cpp
        cli/parser.cpp

        hashing/messageDigest.cpp
        hashing/md5.cpp
        hashing/murmur3.cpp
        hashing/sha1.cpp
        hashing/sha2.cpp
        hashing/sha3.cpp
        )

add_library(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    fmt-header-only)

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
