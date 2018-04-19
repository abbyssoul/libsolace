
set(SOURCE_FILES
        ../include/solace/stringView.hpp
        ../include/solace/string.hpp
        ../include/solace/framework/commandlineParser.hpp


        assert.cpp
        exception.cpp
        error.cpp
        char.cpp

        memoryView.cpp
        immutableMemoryView.cpp
        memoryManager.cpp
        base16.cpp
#        base32.cpp
        base64.cpp
        byteBuffer.cpp
        readBuffer.cpp
        parseUtils.cpp
        uuid.cpp
        string.cpp
        stringBuilder.cpp
        stringView.cpp

        ioobject.cpp
        version.cpp
        path.cpp
        encoder.cpp

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

        process/env.cpp

        cli/helpPrinter.cpp
        framework/commandlineParser.cpp
        framework/application.cpp

        hashing/messageDigest.cpp
        hashing/md5.cpp
        hashing/murmur3.cpp
        hashing/sha1.cpp
        hashing/sha2.cpp
        hashing/sha3.cpp
        )

add_library(${PROJECT_NAME} ${SOURCE_FILES})
#    rt
target_link_libraries(${PROJECT_NAME} PRIVATE
    fmt-header-only)

# Required on Unix OS family to be able to be linked into shared libraries.
#set_target_properties(${PROJECT_NAME}
#                      PROPERTIES POSITION_INDEPENDENT_CODE ON)

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
