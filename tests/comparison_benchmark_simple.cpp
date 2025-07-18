#include "../src/jsonstruct.h"
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include <iostream>

using namespace JsonStruct;

// 简单的基准测试结果结构体
struct BenchmarkResult {
    std::string libraryName;
    std::string operation;
    int iterations;
    double totalTime;
    double averageTime;
    
    BenchmarkResult(const std::string& lib, const std::string& op, int iter, double total, double avg)
        : libraryName(lib), operation(op), iterations(iter), totalTime(total), averageTime(avg) {}
};

// 简单的测试结构体
struct SimpleStruct {
    int id = 12345;
    std::string name = "test_object";
    double value = 98.765;
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    JSON_FIELDS(id, name, value, numbers)
};

// 模拟其他JSON库的性能
class MockJsonLibrary {
public:
    static std::string serialize(const SimpleStruct& obj) {
        // 模拟序列化过程
        std::string result = "{\"id\":" + std::to_string(obj.id) + 
                           ",\"name\":\"" + obj.name + "\"" +
                           ",\"value\":" + std::to_string(obj.value) + 
                           ",\"numbers\":[";
        for (size_t i = 0; i < obj.numbers.size(); ++i) {
            result += std::to_string(obj.numbers[i]);
            if (i < obj.numbers.size() - 1) result += ",";
        }
        result += "]}";
        return result;
    }
    
    static SimpleStruct deserialize(const std::string& jsonStr) {
        // 模拟反序列化过程
        SimpleStruct obj;
        // 这里只是简单返回默认值，实际应该解析JSON
        return obj;
    }
};

// 基准测试函数
BenchmarkResult testJsonStructPerformance(int iterations) {
    SimpleStruct testObj;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        JsonStruct::JsonValue jsonValue(toJson(testObj));
        std::string json = jsonValue.dump();
        SimpleStruct deserialized = fromJson<SimpleStruct>(JsonStruct::JsonValue::parse(json));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    return BenchmarkResult{
        "JsonStruct",
        "Serialize/Deserialize",
        iterations,
        static_cast<double>(duration.count()),
        static_cast<double>(duration.count()) / static_cast<double>(iterations)
    };
}

BenchmarkResult testMockLibraryPerformance(int iterations) {
    SimpleStruct testObj;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        std::string json = MockJsonLibrary::serialize(testObj);
        SimpleStruct deserialized = MockJsonLibrary::deserialize(json);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    return BenchmarkResult{
        "Mock Library",
        "Serialize/Deserialize", 
        iterations,
        static_cast<double>(duration.count()),
        static_cast<double>(duration.count()) / static_cast<double>(iterations)
    };
}

int main() {
    const int iterations = 10000;
    
    std::cout << "=== JsonStruct Performance Comparison ===" << std::endl;
    std::cout << "Running " << iterations << " iterations..." << std::endl;
    
    // 测试JsonStruct性能
    BenchmarkResult jsonStructResult = testJsonStructPerformance(iterations);
    std::cout << std::endl << "JsonStruct Results:" << std::endl;
    std::cout << "Library: " << jsonStructResult.libraryName << std::endl;
    std::cout << "Operation: " << jsonStructResult.operation << std::endl;
    std::cout << "Iterations: " << jsonStructResult.iterations << std::endl;
    std::cout << "Total Time: " << jsonStructResult.totalTime << " microseconds" << std::endl;
    std::cout << "Average Time: " << jsonStructResult.averageTime << " microseconds" << std::endl;
    
    // 测试模拟库性能
    BenchmarkResult mockResult = testMockLibraryPerformance(iterations);
    std::cout << std::endl << "Mock Library Results:" << std::endl;
    std::cout << "Library: " << mockResult.libraryName << std::endl;
    std::cout << "Operation: " << mockResult.operation << std::endl;
    std::cout << "Iterations: " << mockResult.iterations << std::endl;
    std::cout << "Total Time: " << mockResult.totalTime << " microseconds" << std::endl;
    std::cout << "Average Time: " << mockResult.averageTime << " microseconds" << std::endl;
    
    // 性能对比
    std::cout << std::endl << "=== Performance Comparison ===" << std::endl;
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
