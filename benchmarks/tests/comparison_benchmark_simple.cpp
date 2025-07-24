#include "benchmark_framework.h"
#include "json_engine/json_pipeline.h"
#include <iostream>
#include <chrono>

using namespace JsonStruct;

int main() {
    const int iterations = 10000;
    std::cout << "=== JsonStruct Performance Comparison ===" << std::endl;
    std::cout << "Running " << iterations << " iterations..." << std::endl;

    BenchmarkResult jsonStructResult = testJsonStructPerformance(iterations);
    BenchmarkResult mockResult = testMockLibraryPerformance(iterations);

    std::cout << "JsonStruct Results:" << std::endl;
    std::cout << "Library: " << jsonStructResult.libraryName << std::endl;
    std::cout << "Operation: " << jsonStructResult.operation << std::endl;
    std::cout << "Iterations: " << jsonStructResult.iterations << std::endl;
    std::cout << "Total Time: " << jsonStructResult.totalTime << " microseconds" << std::endl;
    std::cout << "Average Time: " << jsonStructResult.averageTime << " microseconds" << std::endl;

    std::cout << "Mock Library Results:" << std::endl;
    std::cout << "Library: " << mockResult.libraryName << std::endl;
    std::cout << "Operation: " << mockResult.operation << std::endl;
    std::cout << "Iterations: " << mockResult.iterations << std::endl;
    std::cout << "Total Time: " << mockResult.totalTime << " microseconds" << std::endl;
    std::cout << "Average Time: " << mockResult.averageTime << " microseconds" << std::endl;

    double jsonStructAvg = jsonStructResult.averageTime;
    double mockAvg = mockResult.averageTime;

    if (jsonStructAvg < mockAvg) {
        double improvement = ((mockAvg - jsonStructAvg) / mockAvg) * 100;
        std::cout << "JsonStruct is " << improvement << "% faster than Mock Library" << std::endl;
    } else {
        double slower = ((jsonStructAvg - mockAvg) / mockAvg) * 100;
        std::cout << "JsonStruct is " << slower << "% slower than Mock Library" << std::endl;
    }

    return 0;
}
