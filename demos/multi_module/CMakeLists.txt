cmake_minimum_required(VERSION 3.10)
project(u8scan_multi_module_demo VERSION 1.0.0 LANGUAGES CXX)

# Set C++11 standard
# Set C++ standard (inherit from parent or default to 11)
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)

# Add the modules
add_subdirectory(module1)
add_subdirectory(module2)

# Add the main executable
add_executable(multi_module_demo main.cpp)
target_link_libraries(multi_module_demo module1 module2)

# Set output directory to match other demos
set_target_properties(multi_module_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
