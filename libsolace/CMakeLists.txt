
set(SOURCE_FILES
        assert.cpp
        memoryView.cpp
        memoryManager.cpp
        byteBuffer.cpp
        char.cpp
        uuid.cpp
        string.cpp
        stringBuilder.cpp
        exception.cpp
        version.cpp
        path.cpp
        error.cpp

        io/ioexception.cpp
        io/file.cpp
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
        io/async/pipe.cpp
        io/async/serialChannel.cpp
        io/async/signalSet.cpp

        process/env.cpp
        framework/commandlineParser.cpp
    )

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} fmt)

# Required on Unix OS family to be able to be linked into shared libraries.
set_target_properties(${PROJECT_NAME}
                      PROPERTIES POSITION_INDEPENDENT_CODE ON)
