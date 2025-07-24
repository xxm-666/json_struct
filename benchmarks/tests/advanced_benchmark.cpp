#include "benchmark_framework.h"
#include "json_engine/json_path.h"
#include "json_engine/json_value.h"
#include "json_engine/json_query_generator.h"
#include "json_engine/json_pipeline.h"
#include <iostream>
#include <chrono>

using namespace JsonStruct;

int main() {
    const int iterations = 10000;
    std::cout << "=== Advanced JsonStruct Benchmark ===" << std::endl;
    std::cout << "Running " << iterations << " iterations..." << std::endl;

    BenchmarkResult jsonPathResult;
    jsonPathResult.libraryName = "JsonStruct";
    jsonPathResult.operation = "JSON Transformation";
    jsonPathResult.iterations = iterations;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        // Simulate a JSON transformation
        JsonPipeline pipeline;
        pipeline.transform([](const JsonValue& value) -> JsonValue {
            // Example transformation: return the input value as-is
            return value;
        });

        JsonValue json = JsonValue::parse("{\"store\": {\"book\": [{\"author\": \"Author1\"}]}}");
        auto result = pipeline.execute(json);
    }
    auto end = std::chrono::high_resolution_clock::now();

    jsonPathResult.totalTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    jsonPathResult.averageTime = jsonPathResult.totalTime / static_cast<double>(iterations);

    std::cout << "Library: " << jsonPathResult.libraryName << std::endl;
    std::cout << "Operation: " << jsonPathResult.operation << std::endl;
    std::cout << "Iterations: " << jsonPathResult.iterations << std::endl;
    std::cout << "Total Time: " << jsonPathResult.totalTime << " microseconds" << std::endl;
    std::cout << "Average Time: " << jsonPathResult.averageTime << " microseconds" << std::endl;

    return 0;
}
