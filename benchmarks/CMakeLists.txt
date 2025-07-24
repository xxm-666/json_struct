cmake_minimum_required(VERSION 3.16)
project(JsonStructBenchmarks LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Paths to JsonStruct source and test framework
set(JSON_STRUCT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(JSON_STRUCT_SRC "${JSON_STRUCT_ROOT}/src")
set(TEST_FRAMEWORK_SRC "${JSON_STRUCT_ROOT}/test_framework")
set(BENCHMARK_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Paths
set(JSON_STRUCT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(JSON_STRUCT_SRC "${JSON_STRUCT_ROOT}/src")
set(TEST_FRAMEWORK_SRC "${JSON_STRUCT_ROOT}/test_framework")

# Include directories
include_directories(
  ${JSON_STRUCT_SRC}
  ${TEST_FRAMEWORK_SRC}
)

# Find dependencies
find_package(Threads REQUIRED)

# JsonStruct core library
add_library(json_struct_benchmark STATIC
  ${JSON_STRUCT_SRC}/json_engine/json_value.cpp
  ${JSON_STRUCT_SRC}/json_engine/json_query_generator.cpp
  ${JSON_STRUCT_SRC}/json_engine/json_path.cpp
  ${JSON_STRUCT_SRC}/json_engine/json_filter.cpp
  ${JSON_STRUCT_SRC}/json_engine/json_pipeline.cpp
  ${JSON_STRUCT_SRC}/json_engine/json_patch.cpp
  ${JSON_STRUCT_SRC}/version.cpp
)
target_compile_features(json_struct_benchmark PUBLIC cxx_std_17)

target_include_directories(json_struct_benchmark PUBLIC ${JSON_STRUCT_SRC})

target_link_libraries(json_struct_benchmark PUBLIC Threads::Threads)

# Test framework library
add_library(test_framework_benchmark STATIC
  ${TEST_FRAMEWORK_SRC}/test_framework.cpp
  ${TEST_FRAMEWORK_SRC}/test_framework.h
)
target_compile_features(test_framework_benchmark PUBLIC cxx_std_17)

target_link_libraries(test_framework_benchmark PUBLIC Threads::Threads)

# Build benchmark executables from tests directory
set(BENCHMARK_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")
file(GLOB BENCHMARK_SOURCES "${BENCHMARK_TEST_DIR}/*.cpp")
foreach(src IN LISTS BENCHMARK_SOURCES)
  get_filename_component(name ${src} NAME_WE)
  add_executable(${name} ${src})
  target_link_libraries(${name} PRIVATE
    test_framework_benchmark
    json_struct_benchmark
    benchmark_framework_lib
    Threads::Threads
  )
  target_compile_features(${name} PRIVATE cxx_std_17)
endforeach()

# Rename the target to avoid conflict
add_library(benchmark_framework_lib STATIC ${CMAKE_CURRENT_SOURCE_DIR}/tests/benchmark_framework.cpp)
target_link_libraries(benchmark_framework_lib PUBLIC Threads::Threads)

# Remove any accidental executable creation for benchmark_framework
# Ensure it is only built as a library
set_target_properties(benchmark_framework_lib PROPERTIES EXCLUDE_FROM_ALL TRUE)
set_target_properties(benchmark_framework PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)

# Advanced benchmark file
# Rename the advanced benchmark target to avoid conflict
add_executable(advanced_benchmark_test ${BENCHMARK_TEST_DIR}/advanced_benchmark.cpp)
target_link_libraries(advanced_benchmark_test PRIVATE
    test_framework_benchmark
    json_struct_benchmark
    benchmark_framework_lib
    Threads::Threads
)
target_compile_features(advanced_benchmark_test PRIVATE cxx_std_17)

# Rename the serialization benchmark target to avoid conflict
add_executable(serialization_benchmark_test ${BENCHMARK_TEST_DIR}/serialization_benchmark.cpp)
target_link_libraries(serialization_benchmark_test PRIVATE
    test_framework_benchmark
    json_struct_benchmark
    benchmark_framework_lib
    Threads::Threads
)
target_compile_features(serialization_benchmark_test PRIVATE cxx_std_17)

# Add the main benchmark entry point
add_executable(jsonstruct_benchmarks ${BENCHMARK_TEST_DIR}/benchmark_main.cpp)
target_link_libraries(jsonstruct_benchmarks PRIVATE
    json_struct_benchmark
    Threads::Threads
)
target_compile_features(jsonstruct_benchmarks PRIVATE cxx_std_17)

