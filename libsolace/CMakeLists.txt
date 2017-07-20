
set(SOURCE_FILES
        assert.cpp
        memoryView.cpp
        immutableMemoryView.cpp
        memoryManager.cpp
        byteBuffer.cpp
        char.cpp
        uuid.cpp
        string.cpp
        stringBuilder.cpp
        exception.cpp
        ioobject.cpp
        version.cpp
        path.cpp
        error.cpp

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
        io/mappedMemory.cpp

        io/async/eventLoop.cpp
        io/async/event.cpp
        io/async/timer.cpp
        io/async/pipe.cpp
        io/async/serialChannel.cpp
        io/async/signalSet.cpp
        io/async/unixSocket.cpp

        process/env.cpp
        framework/commandlineParser.cpp

        hashing/messageDigest.cpp
        hashing/md5.cpp
        hashing/sha2.cpp
        hashing/sha3.cpp
    )

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE
    rt
    fmt)

# Required on Unix OS family to be able to be linked into shared libraries.
set_target_properties(${PROJECT_NAME}
                      PROPERTIES POSITION_INDEPENDENT_CODE ON)
