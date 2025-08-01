#include "benchmark_framework.h"
#include <iostream>

void reportBenchmark(const BenchmarkResult& result) {
    std::cout << "=== Benchmark Report ===" << std::endl;
    std::cout << "Library: " << result.libraryName << std::endl;
    std::cout << "Operation: " << result.operation << std::endl;
    std::cout << "Iterations: " << result.iterations << std::endl;
    std::cout << "Total Time: " << result.totalTime << " microseconds" << std::endl;
    std::cout << "Average Time: " << result.averageTime << " microseconds" << std::endl;
}

int main() {
    BenchmarkResult result;
    result.libraryName = "JsonStruct";
    result.operation = "Serialization";
    result.iterations = 1000;
    result.totalTime = 5000;
    result.averageTime = 5.0;

    reportBenchmark(result);
    return 0;
}
