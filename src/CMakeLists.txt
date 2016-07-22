
set(SOURCE_FILES
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

        io/ioexception.cpp
        io/file.cpp
        io/serial.cpp
        io/serial_list_ports.cpp
        io/signalDispatcher.cpp
        io/selector.cpp
        io/platformFilesystem.cpp
        io/sharedMemory.cpp
        io/mappedMemory.cpp

        process/env.cpp
    )

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} fmt)
