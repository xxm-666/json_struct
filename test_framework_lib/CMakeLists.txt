cmake_minimum_required(VERSION 3.15)
project(TestFramework CXX)

# --- Library Definition ---
# Define a header-only interface library
add_library(test_framework INTERFACE)

# Add a modern CMake alias for better usage
add_library(TestFramework::test_framework ALIAS test_framework)

# Specify the include directory for the library
target_include_directories(test_framework INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# --- Example ---
# Option to control whether examples are built
option(TEST_FRAMEWORK_BUILD_EXAMPLES "Build examples for TestFramework" ON)

if(TEST_FRAMEWORK_BUILD_EXAMPLES)
    # Add the executable for the example
    add_executable(example examples/simple_test.cpp)
    add_executable(test_framework_demo examples/test_framework_demo.cpp)
    # Link the example against the test framework library
    target_link_libraries(example PRIVATE TestFramework::test_framework)
    target_link_libraries(test_framework_demo PRIVATE TestFramework::test_framework)
endif()

# --- Installation ---
# This section allows the library to be installed and found by other projects
# using find_package(TestFramework)

install(TARGETS test_framework EXPORT TestFrameworkTargets)

install(EXPORT TestFrameworkTargets
    FILE
        TestFrameworkTargets.cmake
    NAMESPACE
        TestFramework::
    DESTINATION
        lib/cmake/TestFramework
)

install(
    DIRECTORY include/
    DESTINATION include
)
