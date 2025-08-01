#include "benchmark_framework.h"
#include <iostream>
#include <chrono>

int main() {
    const int iterations = 5000;
    std::cout << "=== JsonStruct Benchmark ===" << std::endl;

    BenchmarkResult result = testJsonStructPerformance(iterations);
    std::cout << "Library: " << result.libraryName << std::endl;
    std::cout << "Operation: " << result.operation << std::endl;
    std::cout << "Iterations: " << result.iterations << std::endl;
    std::cout << "Total Time: " << result.totalTime << " microseconds" << std::endl;
    std::cout << "Average Time: " << result.averageTime << " microseconds" << std::endl;

    return 0;
}
