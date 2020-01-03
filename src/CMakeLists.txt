
set(SOURCE_FILES
        assert.cpp
        exception.cpp
        errorDomain.cpp
        systemErrorDomain.cpp
        error.cpp
        errorString.cpp
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
        string.cpp
        stringBuilder.cpp
        stringView.cpp

        version.cpp
        path.cpp
        encoder.cpp
        env.cpp
        uuid.cpp

        hashing/messageDigest.cpp
        hashing/md5.cpp
        hashing/murmur3.cpp
        hashing/sha1.cpp
        hashing/sha2.cpp
        hashing/sha3.cpp
        )

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC ${CONAN_LIBS})

install(TARGETS ${PROJECT_NAME}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
