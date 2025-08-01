#include "benchmark_framework.h"
#include <iostream>
#include <vector>

int main() {
    const int iterations = 10000;
    std::cout << "=== Memory Benchmark ===" << std::endl;

    BenchmarkResult result = testJsonStructPerformance(iterations);
    std::cout << "Library: " << result.libraryName << std::endl;
    std::cout << "Operation: " << result.operation << std::endl;
    std::cout << "Iterations: " << result.iterations << std::endl;
    std::cout << "Total Time: " << result.totalTime << " microseconds" << std::endl;
    std::cout << "Average Time: " << result.averageTime << " microseconds" << std::endl;

    return 0;
}
