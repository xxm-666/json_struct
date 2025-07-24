# CMake configuration for benchmarks

cmake_minimum_required(VERSION 3.16)

project(JsonStructBenchmarks)

# Add benchmark files
file(GLOB BENCHMARK_SOURCES "*.cpp")

add_executable(JsonStructBenchmarks ${BENCHMARK_SOURCES})

# Link necessary libraries
target_link_libraries(JsonStructBenchmarks PRIVATE JsonStruct)

# Set C++ standard
target_compile_features(JsonStructBenchmarks PRIVATE cxx_std_17)
