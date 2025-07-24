#pragma once
#include <string>
#include <chrono>

struct BenchmarkResult {
    std::string libraryName;
    std::string operation;
    int iterations;
    double totalTime;
    double averageTime;

    BenchmarkResult() : libraryName(""), operation(""), iterations(0), totalTime(0), averageTime(0.0) {}
    BenchmarkResult(const std::string& lib, const std::string& op, int iter, double total, double avg)
        : libraryName(lib), operation(op), iterations(iter), totalTime(total), averageTime(avg) {}
};

// Example benchmark function
BenchmarkResult testJsonStructPerformance(int iterations);
BenchmarkResult testMockLibraryPerformance(int iterations);
