#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <cmath>

namespace BenchmarkFramework {

// 性能测试结果
struct BenchmarkResult {
    std::string name;
    double minTime;     // 最小执行时间 (毫秒)
    double maxTime;     // 最大执行时间 (毫秒)
    double avgTime;     // 平均执行时间 (毫秒)
    double medianTime;  // 中位数执行时间 (毫秒)
    double stdDev;      // 标准差 (毫秒)
    size_t iterations;  // 迭代次数
    size_t memoryUsage; // 内存使用量 (字节)
    double throughput;  // 吞吐量 (操作/秒)
    
    BenchmarkResult(const std::string& n = "") 
        : name(n), minTime(0), maxTime(0), avgTime(0), medianTime(0), 
          stdDev(0), iterations(0), memoryUsage(0), throughput(0) {}
};

// 高精度计时器
class Timer {
public:
    Timer() { reset(); }
    
    void reset() {
        start_ = std::chrono::high_resolution_clock::now();
    }
    
    double elapsed() const {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_);
        return duration.count() / 1000000.0; // 返回毫秒
    }
    
private:
    std::chrono::high_resolution_clock::time_point start_;
};

// 内存使用监控 (简化版本)
class MemoryMonitor {
public:
    static size_t getCurrentMemoryUsage() {
        // 这里可以根据平台实现更精确的内存监控
        // 简化版本返回0，实际项目中可以集成OS特定的API
        return 0;
    }
    
    static size_t getPeakMemoryUsage() {
        return 0;
    }
};

// 基准测试用例
class Benchmark {
public:
    using BenchmarkFunction = std::function<void()>;
    using SetupFunction = std::function<void()>;
    using TeardownFunction = std::function<void()>;
    
    Benchmark(const std::string& name) 
        : name_(name), iterations_(1000), warmupIterations_(10) {}
    
    Benchmark& iterations(size_t count) {
        iterations_ = count;
        return *this;
    }
    
    Benchmark& warmup(size_t count) {
        warmupIterations_ = count;
        return *this;
    }
    
    Benchmark& setup(SetupFunction func) {
        setup_ = func;
        return *this;
    }
    
    Benchmark& teardown(TeardownFunction func) {
        teardown_ = func;
        return *this;
    }
    
    BenchmarkResult run(BenchmarkFunction func) {
        BenchmarkResult result(name_);
        std::vector<double> times;
        times.reserve(iterations_);
        
        // 预热
        for (size_t i = 0; i < warmupIterations_; ++i) {
            if (setup_) setup_();
            func();
            if (teardown_) teardown_();
        }
        
        // 实际测试
        size_t memoryBefore = MemoryMonitor::getCurrentMemoryUsage();
        
        for (size_t i = 0; i < iterations_; ++i) {
            if (setup_) setup_();
            
            Timer timer;
            func();
            double elapsed = timer.elapsed();
            times.push_back(elapsed);
            
            if (teardown_) teardown_();
        }
        
        size_t memoryAfter = MemoryMonitor::getCurrentMemoryUsage();
        
        // 计算统计信息
        result.iterations = iterations_;
        result.minTime = *std::min_element(times.begin(), times.end());
        result.maxTime = *std::max_element(times.begin(), times.end());
        result.avgTime = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        
        // 计算中位数
        std::vector<double> sortedTimes = times;
        std::sort(sortedTimes.begin(), sortedTimes.end());
        if (sortedTimes.size() % 2 == 0) {
            result.medianTime = (sortedTimes[sortedTimes.size()/2 - 1] + 
                               sortedTimes[sortedTimes.size()/2]) / 2.0;
        } else {
            result.medianTime = sortedTimes[sortedTimes.size()/2];
        }
        
        // 计算标准差
        double variance = 0.0;
        for (double time : times) {
            variance += std::pow(time - result.avgTime, 2);
        }
        result.stdDev = std::sqrt(variance / times.size());
        
        // 计算吞吐量 (操作/秒)
        result.throughput = 1000.0 / result.avgTime;
        
        // 内存使用 (简化)
        result.memoryUsage = memoryAfter - memoryBefore;
        
        return result;
    }
    
private:
    std::string name_;
    size_t iterations_;
    size_t warmupIterations_;
    SetupFunction setup_;
    TeardownFunction teardown_;
};

// 基准测试套件
class BenchmarkSuite {
public:
    static BenchmarkSuite& instance() {
        static BenchmarkSuite suite;
        return suite;
    }
    
    void addBenchmark(const std::string& name, std::function<BenchmarkResult()> func) {
        benchmarks_.emplace_back(name, func);
    }
    
    void runAll() {
        std::cout << "\n=== 性能基准测试套件 Performance Benchmark Suite ===" << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        
        results_.clear();
        
        for (auto& [name, func] : benchmarks_) {
            std::cout << "\n正在运行 Running: " << name << "..." << std::endl;
            auto result = func();
            results_.push_back(result);
            printResult(result);
        }
        
        printSummary();
    }
    
    const std::vector<BenchmarkResult>& getResults() const {
        return results_;
    }
    
private:
    std::vector<std::pair<std::string, std::function<BenchmarkResult()>>> benchmarks_;
    std::vector<BenchmarkResult> results_;
    
    void printResult(const BenchmarkResult& result) {
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "  测试名称 Name: " << result.name << std::endl;
        std::cout << "  迭代次数 Iterations: " << result.iterations << std::endl;
        std::cout << "  平均时间 Avg Time: " << result.avgTime << " ms" << std::endl;
        std::cout << "  中位数时间 Median Time: " << result.medianTime << " ms" << std::endl;
        std::cout << "  最小时间 Min Time: " << result.minTime << " ms" << std::endl;
        std::cout << "  最大时间 Max Time: " << result.maxTime << " ms" << std::endl;
        std::cout << "  标准差 Std Dev: " << result.stdDev << " ms" << std::endl;
        std::cout << "  吞吐量 Throughput: " << std::setprecision(0) << result.throughput << " ops/sec" << std::endl;
        if (result.memoryUsage > 0) {
            std::cout << "  内存使用 Memory: " << result.memoryUsage << " bytes" << std::endl;
        }
        std::cout << std::string(60, '-') << std::endl;
    }
    
    void printSummary() {
        std::cout << "\n=== 性能测试总结 Performance Summary ===" << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        
        if (results_.empty()) {
            std::cout << "没有运行任何测试 No benchmarks run." << std::endl;
            return;
        }
        
        // 按吞吐量排序
        auto sortedResults = results_;
        std::sort(sortedResults.begin(), sortedResults.end(), 
                 [](const BenchmarkResult& a, const BenchmarkResult& b) {
                     return a.throughput > b.throughput;
                 });
        
        std::cout << "性能排名 Performance Ranking (按吞吐量 by throughput):" << std::endl;
        for (size_t i = 0; i < sortedResults.size(); ++i) {
            const auto& result = sortedResults[i];
            std::cout << "  " << (i + 1) << ". " << result.name 
                     << " - " << std::fixed << std::setprecision(0) 
                     << result.throughput << " ops/sec" 
                     << " (avg: " << std::setprecision(3) << result.avgTime << " ms)" << std::endl;
        }
        
        std::cout << "\n总共运行了 Total: " << results_.size() << " 个性能测试 benchmarks" << std::endl;
    }
};

// 基准测试注册器
class BenchmarkRegistrar {
public:
    BenchmarkRegistrar(const std::string& name, std::function<BenchmarkResult()> func) {
        BenchmarkSuite::instance().addBenchmark(name, func);
    }
};

} // namespace BenchmarkFramework

// 便利宏定义
#define BENCHMARK(test_name) \
    BenchmarkFramework::BenchmarkResult benchmark_##test_name(); \
    static BenchmarkFramework::BenchmarkRegistrar registrar_##test_name(#test_name, benchmark_##test_name); \
    BenchmarkFramework::BenchmarkResult benchmark_##test_name()

#define RUN_ALL_BENCHMARKS() BenchmarkFramework::BenchmarkSuite::instance().runAll()
