#include "benchmark_framework.h"
#include "json_engine/json_value.h"
#include <chrono>
#include <string>

using namespace JsonStruct;

BenchmarkResult testJsonStructPerformance(int iterations) {
    BenchmarkResult result;
    result.libraryName = "JsonStruct";
    result.operation = "Serialization";
    result.iterations = iterations;

    // Actual serialization test
    JsonValue json = JsonValue::parse("{\"name\": \"John\", \"age\": 30, \"city\": \"New York\"}");
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        std::string serialized = json.toString();
    }
    auto end = std::chrono::high_resolution_clock::now();

    result.totalTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    result.averageTime = result.totalTime / static_cast<double>(iterations);

    return result;
}

BenchmarkResult testMockLibraryPerformance(int iterations) {
    BenchmarkResult result;
    result.libraryName = "MockLibrary";
    result.operation = "Serialization";
    result.iterations = iterations;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        // Simulate mock library serialization logic
    }
    auto end = std::chrono::high_resolution_clock::now();

    result.totalTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    result.averageTime = result.totalTime / static_cast<double>(iterations);

    return result;
}
