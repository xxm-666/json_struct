#include "benchmark_framework.h"
#include "json_engine/json_value.h"
#include <iostream>
#include <chrono>

using namespace JsonStruct;

int main() {
    const int iterations = 10000;
    std::cout << "=== Serialization Benchmark ===" << std::endl;
    std::cout << "Running " << iterations << " iterations..." << std::endl;

    BenchmarkResult serializationResult;
    serializationResult.libraryName = "JsonStruct";
    serializationResult.operation = "Serialization";
    serializationResult.iterations = iterations;

    JsonValue json = JsonValue::parse("{\"name\": \"John\", \"age\": 30, \"city\": \"New York\"}");

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        std::string serialized = json.toString();
    }
    auto end = std::chrono::high_resolution_clock::now();

    serializationResult.totalTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    serializationResult.averageTime = serializationResult.totalTime / static_cast<double>(iterations);

    std::cout << "Library: " << serializationResult.libraryName << std::endl;
    std::cout << "Operation: " << serializationResult.operation << std::endl;
    std::cout << "Iterations: " << serializationResult.iterations << std::endl;
    std::cout << "Total Time: " << serializationResult.totalTime << " microseconds" << std::endl;
    std::cout << "Average Time: " << serializationResult.averageTime << " microseconds" << std::endl;

    // Complex nested JSON structure
    JsonValue complexJson = JsonValue::parse(R"({
        "user": {
            "id": 123,
            "name": "Alice",
            "roles": ["admin", "editor"],
            "profile": {
                "age": 29,
                "address": {
                    "city": "Wonderland",
                    "zip": "12345"
                }
            }
        },
        "logs": [
            {"action": "login", "timestamp": "2025-07-24T10:00:00Z"},
            {"action": "update", "timestamp": "2025-07-24T10:05:00Z"}
        ]
    })");

    BenchmarkResult complexSerializationResult;
    complexSerializationResult.libraryName = "JsonStruct";
    complexSerializationResult.operation = "Complex Serialization";
    complexSerializationResult.iterations = iterations;

    auto startComplex = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        std::string serialized = complexJson.toString();
    }
    auto endComplex = std::chrono::high_resolution_clock::now();

    complexSerializationResult.totalTime = std::chrono::duration_cast<std::chrono::microseconds>(endComplex - startComplex).count();
    complexSerializationResult.averageTime = complexSerializationResult.totalTime / static_cast<double>(iterations);

    std::cout << "\nLibrary: " << complexSerializationResult.libraryName << std::endl;
    std::cout << "Operation: " << complexSerializationResult.operation << std::endl;
    std::cout << "Iterations: " << complexSerializationResult.iterations << std::endl;
    std::cout << "Total Time: " << complexSerializationResult.totalTime << " microseconds" << std::endl;
    std::cout << "Average Time: " << complexSerializationResult.averageTime << " microseconds" << std::endl;

    return 0;
}
