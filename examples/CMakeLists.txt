# Build examples

# Example of using VFS as ram disk
set(EXAMPLE_APPFRAMEWORK_SOURCE_FILES ds_parser.cpp)
add_executable(ds_parser ${EXAMPLE_APPFRAMEWORK_SOURCE_FILES})
target_link_libraries(ds_parser PUBLIC ${PROJECT_NAME} ${CONAN_LIBS})


add_custom_target(examples
    DEPENDS ds_parser)
