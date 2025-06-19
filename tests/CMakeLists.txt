# Tests CMakeLists.txt

# Find the utest header (it's included in our project)
# No need to find external packages since utest is header-only and included

# U8Scan Scanning test executable
add_executable(u8scan_scanning_test u8scan_scanning_test.cpp)
target_link_libraries(u8scan_scanning_test PRIVATE u8scan::u8scan)
set_target_properties(u8scan_scanning_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# U8Scan STL test executable
add_executable(u8scan_stl_test u8scan_stl_test.cpp)
target_link_libraries(u8scan_stl_test PRIVATE u8scan::u8scan)
set_target_properties(u8scan_stl_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# U8Scan Emoji test executable
add_executable(u8scan_emoji_test u8scan_emoji_test.cpp)
target_link_libraries(u8scan_emoji_test PRIVATE u8scan::u8scan)
set_target_properties(u8scan_emoji_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# U8Scan Copy test executable
add_executable(u8scan_copy_test u8scan_copy_test.cpp)
target_link_libraries(u8scan_copy_test PRIVATE u8scan::u8scan)
set_target_properties(u8scan_copy_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# U8Scan Access test executable (tests for at, empty, front, back functions)
add_executable(u8scan_access_test u8scan_access_test.cpp)
target_link_libraries(u8scan_access_test PRIVATE u8scan::u8scan)
set_target_properties(u8scan_access_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Add tests to CTest
add_test(NAME U8ScanScanningTest COMMAND u8scan_scanning_test)
add_test(NAME U8ScanSTLTest COMMAND u8scan_stl_test)
add_test(NAME U8ScanEmojiTest COMMAND u8scan_emoji_test)
add_test(NAME U8ScanCopyTest COMMAND u8scan_copy_test)
add_test(NAME U8ScanAccessTest COMMAND u8scan_access_test)

# Test discovery for better integration with IDEs
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.10)
    include(GoogleTest)
    # Since we're not using Google Test, we'll use a simple approach
    # The tests will run as single units
endif()

# Custom target for running tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS u8scan_scanning_test u8scan_stl_test u8scan_emoji_test u8scan_copy_test u8scan_access_test
    COMMENT "Running all tests"
)

# Add compile definitions for debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(u8scan_scanning_test PRIVATE DEBUG=1)
    target_compile_definitions(u8scan_stl_test PRIVATE DEBUG=1)
    target_compile_definitions(u8scan_emoji_test PRIVATE DEBUG=1)
    target_compile_definitions(u8scan_copy_test PRIVATE DEBUG=1)
    target_compile_definitions(u8scan_access_test PRIVATE DEBUG=1)
endif()

message(STATUS "Test configuration:")
message(STATUS "  Test executables: u8scan_scanning_test, u8scan_stl_test, u8scan_emoji_test, u8scan_copy_test, u8scan_access_test")
message(STATUS "  Output directory: ${CMAKE_BINARY_DIR}/bin")
