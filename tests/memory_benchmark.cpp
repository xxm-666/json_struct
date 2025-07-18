#include "../src/jsonstruct.h"
#include "../src/json_engine/json_path.h"
#include "../src/json_engine/json_value.h"
#include "benchmark_framework.h"
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#elif defined(__linux__)
#include <unistd.h>
#include <fstream>
#elif defined(__APPLE__)
#include <mach/mach.h>
#endif

using namespace JsonStruct;
using namespace jsonvalue_jsonpath;
using namespace BenchmarkFramework;

// 跨平台内存监控类
class MemoryProfiler {
public:
    struct MemoryInfo {
        size_t totalMemory = 0;      // 总内存使用 (字节)
        size_t peakMemory = 0;       // 峰值内存使用 (字节)
        size_t availableMemory = 0;  // 可用内存 (字节)
        
        void print() const {
            std::cout << "  总内存使用 Total Memory: " << (totalMemory / 1024) << " KB" << std::endl;
            std::cout << "  峰值内存 Peak Memory: " << (peakMemory / 1024) << " KB" << std::endl;
            std::cout << "  可用内存 Available Memory: " << (availableMemory / 1024 / 1024) << " MB" << std::endl;
        }
    };
    
    static MemoryInfo getCurrentMemoryInfo() {
        MemoryInfo info;
        
#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            info.totalMemory = pmc.WorkingSetSize;
            info.peakMemory = pmc.PeakWorkingSetSize;
        }
        
        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(memStatus);
        if (GlobalMemoryStatusEx(&memStatus)) {
            info.availableMemory = memStatus.ullAvailPhys;
        }
        
#elif defined(__linux__)
        std::ifstream statusFile("/proc/self/status");
        std::string line;
        while (std::getline(statusFile, line)) {
            if (line.substr(0, 6) == "VmRSS:") {
                info.totalMemory = std::stoul(line.substr(7)) * 1024; // KB to bytes
            } else if (line.substr(0, 9) == "VmHWM:") {
                info.peakMemory = std::stoul(line.substr(10)) * 1024;
            }
        }
        
        std::ifstream meminfoFile("/proc/meminfo");
        while (std::getline(meminfoFile, line)) {
            if (line.substr(0, 13) == "MemAvailable:") {
                info.availableMemory = std::stoul(line.substr(14)) * 1024;
                break;
            }
        }
        
#elif defined(__APPLE__)
        mach_task_basic_info info_data;
        mach_msg_type_number_t info_count = MACH_TASK_BASIC_INFO_COUNT;
        if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, 
                     (task_info_t)&info_data, &info_count) == KERN_SUCCESS) {
            info.totalMemory = info_data.resident_size;
        }
#endif
        
        return info;
    }
    
    static void printMemoryComparison(const MemoryInfo& before, const MemoryInfo& after, 
                                    const std::string& operation) {
        std::cout << "\n内存使用分析 Memory Usage Analysis - " << operation << std::endl;
        std::cout << std::string(60, '-') << std::endl;
        
        std::cout << "操作前 Before:" << std::endl;
        before.print();
        
        std::cout << "操作后 After:" << std::endl;
        after.print();
        
        long long memoryDiff = static_cast<long long>(after.totalMemory) - 
                              static_cast<long long>(before.totalMemory);
        std::cout << "内存变化 Memory Change: " << (memoryDiff / 1024) << " KB" << std::endl;
        
        if (memoryDiff > 0) {
            std::cout << "内存增加 Memory Increased" << std::endl;
        } else if (memoryDiff < 0) {
            std::cout << "内存减少 Memory Decreased" << std::endl;
        } else {
            std::cout << "内存无变化 No Memory Change" << std::endl;
        }
    }
};

// 测试数据结构
struct MemoryTestStruct {
    std::vector<std::string> largeStrings;
    std::map<std::string, std::vector<double>> dataMap;
    std::vector<std::vector<int>> matrix;
    
    JSON_AUTO(largeStrings, dataMap, matrix)
};

// 生成大内存消耗的测试数据
MemoryTestStruct generateLargeMemoryData() {
    MemoryTestStruct data;
    
    // 生成大字符串数组
    data.largeStrings.reserve(1000);
    for (int i = 0; i < 1000; ++i) {
        std::string largeStr(1000, 'A' + (i % 26)); // 每个字符串1KB
        data.largeStrings.push_back(largeStr);
    }
    
    // 生成数据映射
    for (int i = 0; i < 100; ++i) {
        std::vector<double> values;
        values.reserve(100);
        for (int j = 0; j < 100; ++j) {
            values.push_back(i * 100.0 + j);
        }
        data.dataMap["dataset_" + std::to_string(i)] = std::move(values);
    }
    
    // 生成矩阵数据
    data.matrix.reserve(100);
    for (int i = 0; i < 100; ++i) {
        std::vector<int> row;
        row.reserve(100);
        for (int j = 0; j < 100; ++j) {
            row.push_back(i * 100 + j);
        }
        data.matrix.push_back(std::move(row));
    }
    
    return data;
}

// === 内存使用测试 ===

BENCHMARK(Memory_Serialization_Footprint) {
    auto memBefore = MemoryProfiler::getCurrentMemoryInfo();
    
    return Benchmark("序列化内存占用")
        .iterations(100)
        .warmup(10)
        .setup([&]() {
            // 记录每次操作前的内存
        })
        .run([&]() {
            MemoryTestStruct data = generateLargeMemoryData();
            auto json = data.toJson();
            std::string jsonStr = json.dump();
            
            // 确保不被优化掉
            volatile size_t len = jsonStr.length();
            (void)len;
            
            // 清理数据，测试内存回收
            jsonStr.clear();
        });
}

BENCHMARK(Memory_Deserialization_Footprint) {
    MemoryTestStruct original = generateLargeMemoryData();
    auto jsonVal = original.toJson();
    std::string jsonStr = jsonVal.dump();
    
    return Benchmark("反序列化内存占用")
        .iterations(50)
        .warmup(5)
        .run([&]() {
            MemoryTestStruct restored;
            restored.fromJson(JsonValue::parse(jsonStr));
            
            // 访问数据确保完全加载
            volatile size_t strCount = restored.largeStrings.size();
            volatile size_t mapCount = restored.dataMap.size();
            volatile size_t matrixCount = restored.matrix.size();
            (void)strCount; (void)mapCount; (void)matrixCount;
        });
}

BENCHMARK(Memory_JSONPath_Query_Footprint) {
    MemoryTestStruct data = generateLargeMemoryData();
    JsonValue jsonValue = data.toJson();
    
    return Benchmark("JSONPath查询内存占用")
        .iterations(100)
        .warmup(10)
        .run([&]() {
            // 执行多个查询操作
            auto results1 = selectAll(jsonValue, "$.largeStrings[*]");
            auto results2 = selectAll(jsonValue, "$.dataMap.*[*]");
            auto results3 = selectAll(jsonValue, "$.matrix[*][*]");
            
            volatile size_t total = results1.size() + results2.size() + results3.size();
            (void)total;
        });
}

// === 内存泄漏检测测试 ===

void memoryLeakTest() {
    std::cout << "\n=== 内存泄漏检测测试 Memory Leak Detection Test ===" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    auto initialMemory = MemoryProfiler::getCurrentMemoryInfo();
    std::cout << "初始内存状态 Initial Memory State:" << std::endl;
    initialMemory.print();
    
    // 执行大量操作
    std::cout << "\n执行 1000 次序列化/反序列化操作..." << std::endl;
    std::cout << "Performing 1000 serialization/deserialization operations..." << std::endl;
    
    for (int i = 0; i < 1000; ++i) {
        MemoryTestStruct data = generateLargeMemoryData();
        auto jsonVal = data.toJson();
        std::string json = jsonVal.dump();
        MemoryTestStruct restored;
        restored.fromJson(JsonValue::parse(json));
        
        // 定期输出进度
        if (i % 200 == 0) {
            auto currentMemory = MemoryProfiler::getCurrentMemoryInfo();
            std::cout << "迭代 Iteration " << i << " - 当前内存 Current Memory: " 
                     << (currentMemory.totalMemory / 1024) << " KB" << std::endl;
        }
    }
    
    // 强制垃圾回收 (在支持的编译器上)
    std::cout << "\n等待内存回收..." << std::endl;
    std::cout << "Waiting for memory cleanup..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    auto finalMemory = MemoryProfiler::getCurrentMemoryInfo();
    
    MemoryProfiler::printMemoryComparison(initialMemory, finalMemory, "内存泄漏检测");
    
    long long memoryDiff = static_cast<long long>(finalMemory.totalMemory) - 
                          static_cast<long long>(initialMemory.totalMemory);
    
    if (std::abs(memoryDiff) < 10 * 1024 * 1024) { // 小于10MB被认为是正常的
        std::cout << "✅ 内存泄漏检测通过 Memory leak test PASSED" << std::endl;
        std::cout << "内存使用稳定，无明显泄漏 Memory usage stable, no significant leaks" << std::endl;
    } else {
        std::cout << "⚠️  检测到可能的内存问题 Potential memory issue detected" << std::endl;
        std::cout << "建议进一步调查 Further investigation recommended" << std::endl;
    }
}

// === 内存效率对比测试 ===

void memoryEfficiencyComparison() {
    std::cout << "\n=== 内存效率对比 Memory Efficiency Comparison ===" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    struct TestCase {
        std::string name;
        std::function<void()> operation;
    };
    
    std::vector<TestCase> testCases = {
        {"小对象序列化 Small Object Serialization", []() {
            struct SmallStruct { int a; std::string b; JSON_AUTO(a, b) };
            SmallStruct obj{42, "test"};
            for (int i = 0; i < 1000; ++i) {
                auto json = obj.toJson();
                std::string jsonStr = json.dump();
                volatile size_t len = jsonStr.length();
                (void)len;
            }
        }},
        
        {"中等对象序列化 Medium Object Serialization", []() {
            struct MediumStruct { 
                std::vector<int> nums; 
                std::map<std::string, double> data;
                JSON_AUTO(nums, data)
            };
            MediumStruct obj;
            obj.nums = std::vector<int>(100, 42);
            obj.data = {{"key1", 1.0}, {"key2", 2.0}, {"key3", 3.0}};
            
            for (int i = 0; i < 100; ++i) {
                auto json = obj.toJson();
                std::string jsonStr = json.dump();
                volatile size_t len = jsonStr.length();
                (void)len;
            }
        }},
        
        {"大对象序列化 Large Object Serialization", []() {
            MemoryTestStruct obj = generateLargeMemoryData();
            for (int i = 0; i < 10; ++i) {
                auto json = obj.toJson();
                std::string jsonStr = json.dump();
                volatile size_t len = jsonStr.length();
                (void)len;
            }
        }},
        
        {"JSONPath查询 JSONPath Queries", []() {
            MemoryTestStruct obj = generateLargeMemoryData();
            JsonValue jsonValue = obj.toJson();
            
            for (int i = 0; i < 50; ++i) {
                auto results = selectAll(jsonValue, "$.largeStrings[*]");
                volatile size_t count = results.size();
                (void)count;
            }
        }}
    };
    
    for (const auto& testCase : testCases) {
        auto memBefore = MemoryProfiler::getCurrentMemoryInfo();
        
        std::cout << "\n测试 Testing: " << testCase.name << std::endl;
        testCase.operation();
        
        auto memAfter = MemoryProfiler::getCurrentMemoryInfo();
        MemoryProfiler::printMemoryComparison(memBefore, memAfter, testCase.name);
    }
}

int main() {
    std::cout << "JsonStruct 内存使用性能分析" << std::endl;
    std::cout << "JsonStruct Memory Usage Performance Analysis" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    // 显示系统信息
    auto systemMemory = MemoryProfiler::getCurrentMemoryInfo();
    std::cout << "系统内存信息 System Memory Info:" << std::endl;
    systemMemory.print();
    
    // 运行基准测试
    RUN_ALL_BENCHMARKS();
    
    // 内存泄漏检测
    memoryLeakTest();
    
    // 内存效率对比
    memoryEfficiencyComparison();
    
    std::cout << "\n=== 内存使用总结 Memory Usage Summary ===" << std::endl;
    std::cout << "1. JsonStruct采用RAII设计，自动管理内存资源" << std::endl;
    std::cout << "2. 移动语义优化减少不必要的内存复制" << std::endl;
    std::cout << "3. JSONPath查询采用引用返回，避免额外内存分配" << std::endl;
    std::cout << "4. 编译时优化减少运行时内存开销" << std::endl;
    std::cout << std::endl;
    std::cout << "建议 Recommendations:" << std::endl;
    std::cout << "- 对于大数据处理，考虑使用流式处理" << std::endl;
    std::cout << "- 使用JSONPath的selectAllMutable避免额外复制" << std::endl;
    std::cout << "- 在Debug模式下可能有额外的内存开销" << std::endl;
    
    return 0;
}
