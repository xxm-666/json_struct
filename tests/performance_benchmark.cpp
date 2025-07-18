#include "../src/jsonstruct.h"
#include "../src/json_engine/json_path.h"
#include "../src/json_engine/json_value.h"
#include "benchmark_framework.h"
#include <vector>
#include <string>
#include <map>
#include <random>
#include <memory>

using namespace JsonStruct;
using namespace jsonvalue_jsonpath;
using namespace BenchmarkFramework;

// 测试数据结构定义
struct SimpleStruct {
    int id;
    std::string name;
    double value;
    
    JSON_AUTO(id, name, value)
};

struct ComplexStruct {
    int id;
    std::string name;
    double value;
    std::vector<int> numbers;
    std::map<std::string, std::string> properties;
    std::vector<SimpleStruct> nested;
    
    JSON_AUTO(id, name, value, numbers, properties, nested)
};

struct LargeStruct {
    std::vector<ComplexStruct> data;
    std::map<std::string, std::vector<double>> metrics;
    std::string description;
    
    JSON_AUTO(data, metrics, description)
};

// 测试数据生成器
class TestDataGenerator {
public:
    static SimpleStruct generateSimpleStruct(int seed = 0) {
        std::mt19937 gen(seed);
        std::uniform_int_distribution<> intDist(1, 1000);
        std::uniform_real_distribution<> doubleDist(0.0, 100.0);
        
        SimpleStruct s;
        s.id = intDist(gen);
        s.name = "test_object_" + std::to_string(seed);
        s.value = doubleDist(gen);
        return s;
    }
    
    static ComplexStruct generateComplexStruct(int seed = 0, size_t arraySize = 10) {
        std::mt19937 gen(seed);
        std::uniform_int_distribution<> intDist(1, 1000);
        std::uniform_real_distribution<> doubleDist(0.0, 100.0);
        
        ComplexStruct c;
        c.id = intDist(gen);
        c.name = "complex_object_" + std::to_string(seed);
        c.value = doubleDist(gen);
        
        // 生成数组数据
        c.numbers.reserve(arraySize);
        for (size_t i = 0; i < arraySize; ++i) {
            c.numbers.push_back(intDist(gen));
        }
        
        // 生成属性字典
        for (size_t i = 0; i < 5; ++i) {
            c.properties["key_" + std::to_string(i)] = "value_" + std::to_string(intDist(gen));
        }
        
        // 生成嵌套结构
        for (size_t i = 0; i < 3; ++i) {
            c.nested.push_back(generateSimpleStruct(seed + i));
        }
        
        return c;
    }
    
    static LargeStruct generateLargeStruct(int seed = 0, size_t dataSize = 100) {
        LargeStruct large;
        large.description = "Large test structure with " + std::to_string(dataSize) + " complex objects";
        
        // 生成大量复杂结构
        large.data.reserve(dataSize);
        for (size_t i = 0; i < dataSize; ++i) {
            large.data.push_back(generateComplexStruct(seed + i, 20));
        }
        
        // 生成指标数据
        std::mt19937 gen(seed);
        std::uniform_real_distribution<> doubleDist(0.0, 100.0);
        
        for (const std::string& metric : {"cpu_usage", "memory_usage", "disk_io", "network_io"}) {
            std::vector<double> values;
            values.reserve(50);
            for (size_t i = 0; i < 50; ++i) {
                values.push_back(doubleDist(gen));
            }
            large.metrics[metric] = std::move(values);
        }
        
        return large;
    }
    
    static std::string generateLargeJsonString(size_t objectCount = 1000) {
        std::string result = "{\"objects\":[";
        for (size_t i = 0; i < objectCount; ++i) {
            if (i > 0) result += ",";
            result += "{\"id\":" + std::to_string(i) + 
                     ",\"name\":\"object_" + std::to_string(i) + "\"" +
                     ",\"data\":[1,2,3,4,5]" +
                     ",\"properties\":{\"type\":\"test\",\"category\":\"benchmark\"}}";
        }
        result += "]}";
        return result;
    }
};

// === 序列化性能测试 ===

BENCHMARK(SimpleStruct_Serialization) {
    SimpleStruct testData = TestDataGenerator::generateSimpleStruct(42);
    
    return Benchmark("SimpleStruct序列化")
        .iterations(10000)
        .warmup(100)
        .run([&]() {
            auto json = testData.toJson();
            std::string jsonStr = json.dump();
            // 防止编译器优化掉
            volatile size_t len = jsonStr.length();
            (void)len;
        });
}

BENCHMARK(ComplexStruct_Serialization) {
    ComplexStruct testData = TestDataGenerator::generateComplexStruct(42, 50);
    
    return Benchmark("ComplexStruct序列化")
        .iterations(5000)
        .warmup(50)
        .run([&]() {
            auto json = testData.toJson();
            std::string jsonStr = json.dump();
            volatile size_t len = jsonStr.length();
            (void)len;
        });
}

BENCHMARK(LargeStruct_Serialization) {
    LargeStruct testData = TestDataGenerator::generateLargeStruct(42, 100);
    
    return Benchmark("LargeStruct序列化")
        .iterations(100)
        .warmup(10)
        .run([&]() {
            auto json = testData.toJson();
            std::string jsonStr = json.dump();
            volatile size_t len = jsonStr.length();
            (void)len;
        });
}

// === 反序列化性能测试 ===

BENCHMARK(SimpleStruct_Deserialization) {
    SimpleStruct original = TestDataGenerator::generateSimpleStruct(42);
    auto jsonVal = original.toJson();
    std::string jsonStr = jsonVal.dump();
    
    return Benchmark("SimpleStruct反序列化")
        .iterations(10000)
        .warmup(100)
        .run([&]() {
            SimpleStruct restored;
            restored.fromJson(JsonValue::parse(jsonStr));
            volatile int id = restored.id;
            (void)id;
        });
}

BENCHMARK(ComplexStruct_Deserialization) {
    ComplexStruct original = TestDataGenerator::generateComplexStruct(42, 50);
    auto jsonVal = original.toJson();
    std::string jsonStr = jsonVal.dump();
    
    return Benchmark("ComplexStruct反序列化")
        .iterations(5000)
        .warmup(50)
        .run([&]() {
            ComplexStruct restored;
            restored.fromJson(JsonValue::parse(jsonStr));
            volatile int id = restored.id;
            (void)id;
        });
}

BENCHMARK(LargeStruct_Deserialization) {
    LargeStruct original = TestDataGenerator::generateLargeStruct(42, 100);
    auto jsonVal = original.toJson();
    std::string jsonStr = jsonVal.dump();
    
    return Benchmark("LargeStruct反序列化")
        .iterations(50)
        .warmup(5)
        .run([&]() {
            LargeStruct restored;
            restored.fromJson(JsonValue::parse(jsonStr));
            volatile size_t size = restored.data.size();
            (void)size;
        });
}

// === JSONPath查询性能测试 ===

BENCHMARK(JSONPath_Simple_Query) {
    LargeStruct testData = TestDataGenerator::generateLargeStruct(42, 200);
    JsonValue jsonValue = testData.toJson();
    
    return Benchmark("JSONPath简单查询")
        .iterations(1000)
        .warmup(10)
        .run([&]() {
            auto results = selectAll(jsonValue, "$.data[*].id");
            volatile size_t count = results.size();
            (void)count;
        });
}

BENCHMARK(JSONPath_Complex_Query) {
    LargeStruct testData = TestDataGenerator::generateLargeStruct(42, 200);
    JsonValue jsonValue = testData.toJson();
    
    return Benchmark("JSONPath复杂查询")
        .iterations(500)
        .warmup(5)
        .run([&]() {
            auto results = selectAll(jsonValue, "$.data[?(@.id > 500)].name");
            volatile size_t count = results.size();
            (void)count;
        });
}

BENCHMARK(JSONPath_Recursive_Query) {
    LargeStruct testData = TestDataGenerator::generateLargeStruct(42, 100);
    JsonValue jsonValue = testData.toJson();
    
    return Benchmark("JSONPath递归查询")
        .iterations(200)
        .warmup(5)
        .run([&]() {
            auto results = selectAll(jsonValue, "$..id");
            volatile size_t count = results.size();
            (void)count;
        });
}

BENCHMARK(JSONPath_Mutable_Operations) {
    LargeStruct testData = TestDataGenerator::generateLargeStruct(42, 100);
    
    return Benchmark("JSONPath可变操作")
        .iterations(100)
        .warmup(5)
        .setup([&]() {
            // 每次重新创建测试数据
            testData = TestDataGenerator::generateLargeStruct(42, 100);
        })
        .run([&]() {
            JsonValue jsonValue = testData.toJson();
            auto results = selectAllMutable(jsonValue, "$.data[*]");
            for (auto& result : results) {
                result.get()["processed"] = JsonValue(true);
            }
            volatile size_t count = results.size();
            (void)count;
        });
}

// === 大数据量处理性能测试 ===

BENCHMARK(Large_JSON_Parsing) {
    std::string largeJson = TestDataGenerator::generateLargeJsonString(2000);
    
    return Benchmark("大JSON解析")
        .iterations(100)
        .warmup(5)
        .run([&]() {
            JsonValue parsed = JsonValue::parse(largeJson);
            volatile bool isObject = parsed.isObject();
            (void)isObject;
        });
}

BENCHMARK(STL_Container_Serialization) {
    std::vector<std::map<std::string, std::vector<int>>> complexContainer;
    
    // 生成复杂容器数据
    for (int i = 0; i < 100; ++i) {
        std::map<std::string, std::vector<int>> innerMap;
        for (int j = 0; j < 10; ++j) {
            std::vector<int> vec;
            for (int k = 0; k < 20; ++k) {
                vec.push_back(i * 100 + j * 10 + k);
            }
            innerMap["key_" + std::to_string(j)] = std::move(vec);
        }
        complexContainer.push_back(std::move(innerMap));
    }
    
    return Benchmark("STL容器序列化")
        .iterations(200)
        .warmup(10)
        .run([&]() {
            JsonValue json = toJsonValue(complexContainer);
            volatile bool isArray = json.isArray();
            (void)isArray;
        });
}

// === 内存效率测试 ===

BENCHMARK(Memory_Efficiency_Test) {
    std::vector<ComplexStruct> testData;
    for (int i = 0; i < 1000; ++i) {
        testData.push_back(TestDataGenerator::generateComplexStruct(i, 10));
    }
    
    return Benchmark("内存效率测试")
        .iterations(50)
        .warmup(5)
        .run([&]() {
            // 序列化
            std::vector<JsonValue> jsonValues;
            jsonValues.reserve(testData.size());
            
            for (const auto& item : testData) {
                jsonValues.push_back(item.toJson());
            }
            
            // 反序列化
            std::vector<ComplexStruct> restored;
            restored.reserve(jsonValues.size());
            
            for (const auto& jsonVal : jsonValues) {
                ComplexStruct temp;
                temp.fromJson(jsonVal);
                restored.push_back(std::move(temp));
            }
            
            volatile size_t count = restored.size();
            (void)count;
        });
}

// 主函数
int main() {
    std::cout << "JsonStruct 性能基准测试套件" << std::endl;
    std::cout << "Performance Benchmark Suite for JsonStruct" << std::endl;
    std::cout << "版本 Version: 1.2.0-dev" << std::endl;
    std::cout << "编译器 Compiler: " << 
#ifdef _MSC_VER
        "MSVC " << _MSC_VER
#elif defined(__GNUC__)
        "GCC " << __GNUC__ << "." << __GNUC_MINOR__
#elif defined(__clang__)
        "Clang " << __clang_major__ << "." << __clang_minor__
#else
        "Unknown"
#endif
        << std::endl;
    
    std::cout << "构建类型 Build Type: " << 
#ifdef NDEBUG
        "Release"
#else
        "Debug"
#endif
        << std::endl;
    
    std::cout << "测试开始时间 Test Start Time: " << __DATE__ << " " << __TIME__ << std::endl;
    
    RUN_ALL_BENCHMARKS();
    
    return 0;
}
