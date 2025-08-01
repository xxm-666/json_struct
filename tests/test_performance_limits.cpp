#include "../test_framework/test_framework.h"
#include "../src/type_registry/registry_core.h"
#include "../src/type_registry/auto_serializer.h"
#include "../src/json_engine/json_value.h"
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <iostream>
#include <iomanip>

using namespace JsonStruct;

// 定义用于性能测试的数据结构
struct PerformanceData {
    JSON_AUTO(id, name, value, timestamp, tags)
    
    int id;
    std::string name;
    double value;
    long long timestamp;
    std::map<std::string, std::string> tags;
};

// 生成大量测试数据的辅助函数
std::vector<PerformanceData> generateLargeDataset(size_t count) {
    std::vector<PerformanceData> data;
    data.reserve(count);
    
    for (size_t i = 0; i < count; ++i) {
        PerformanceData item;
        item.id = static_cast<int>(i);
        item.name = "Item_" + std::to_string(i);
        item.value = static_cast<double>(i) * 1.5;
        item.timestamp = static_cast<long long>(i) * 1000;
        item.tags = {{"source", "generator"}, {"version", "1.0"}, {"batch", std::to_string(i / 1000)}};
        data.emplace_back(std::move(item));
    }
    
    return data;
}

// 测试序列化大量数据的性能
TEST(Performance_SerializeMillionObjects) {
    const size_t dataSize = 1000000; // 100万条数据
    
    std::cout << "Generating " << dataSize << " objects for serialization test..." << std::endl;
    auto data = generateLargeDataset(dataSize);
    
    // 记录开始时间
    auto start = std::chrono::high_resolution_clock::now();
    
    // 执行序列化
    // JsonValue json = toJsonValue(data);
    std::vector<JsonValue> jsonArr;
    jsonArr.reserve(data.size());
    for (const auto& item : data) {
        jsonArr.push_back(toJsonValue(item));
    }
    JsonValue json(jsonArr);
    
    // 记录结束时间
    auto end = std::chrono::high_resolution_clock::now();
    
    // 计算耗时
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 获取JSON字符串大小
    std::string jsonString = json.dump();
    size_t jsonSize = jsonString.size();
    
    std::cout << "Serialized " << dataSize << " objects in " << duration.count() << " ms" << std::endl;
    std::cout << "Generated JSON size: " << jsonSize << " bytes" << std::endl;
    
    // 验证序列化成功
    ASSERT_TRUE(json.isArray());
    ASSERT_EQ(json.size(), dataSize);
    
    // 性能断言（根据实际情况调整阈值）
    ASSERT_TRUE(duration.count() < 5000); // 序列化应该在5秒内完成
}

// 测试解析大量数据的性能
TEST(Performance_ParseMillionObjects) {
    const size_t dataSize = 1000000; // 100万条数据
    
    std::cout << "Generating " << dataSize << " objects for parsing test..." << std::endl;
    auto data = generateLargeDataset(dataSize);
    // JsonValue json(data);
    std::vector<JsonValue> jsonArr;
    jsonArr.reserve(data.size());
    for (const auto& item : data) {
        jsonArr.push_back(toJsonValue(item));
    }
    JsonValue json(jsonArr);
    std::string jsonString = json.dump();
    
    // 清理内存以获得更准确的内存测量
    data.clear();
    json = JsonValue();
    
    // 记录开始时间
    auto start = std::chrono::high_resolution_clock::now();
    
    // 执行解析
    JsonValue parsed = JsonValue::parse(jsonString);
    
    // 记录结束时间
    auto end = std::chrono::high_resolution_clock::now();
    
    // 计算耗时
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Parsed " << dataSize << " objects in " << duration.count() << " ms" << std::endl;
    
    // 验证解析成功
    ASSERT_TRUE(parsed.isArray());
    ASSERT_EQ(parsed.size(), dataSize);
    
    // 性能断言（根据实际情况调整阈值）
    ASSERT_TRUE(duration.count() <= 5000); // 解析应该在5秒内完成
}

// 测试大型数组的序列化性能
TEST(Performance_SerializeLargeArray) {
    const size_t arraySize = 10000000; // 1000万个元素
    
    std::cout << "Creating array with " << arraySize << " elements..." << std::endl;
    std::vector<double> largeArray;
    largeArray.reserve(arraySize);
    
    for (size_t i = 0; i < arraySize; ++i) {
        largeArray.push_back(static_cast<double>(i) * 3.14159);
    }
    
    // 记录开始时间
    auto start = std::chrono::high_resolution_clock::now();
    
    // 执行序列化
    JsonValue json(largeArray);
    
    // 记录结束时间
    auto end = std::chrono::high_resolution_clock::now();
    
    // 计算耗时
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 获取JSON字符串大小
    std::string jsonString = json.dump();
    size_t jsonSize = jsonString.size();
    
    std::cout << "Serialized array with " << arraySize << " elements in " << duration.count() << " ms" << std::endl;
    std::cout << "Generated JSON size: " << jsonSize << " bytes" << std::endl;
    
    // 验证序列化成功
    ASSERT_TRUE(json.isArray());
    ASSERT_EQ(json.size(), arraySize);
    
    // 性能断言（根据实际情况调整阈值）
    ASSERT_TRUE(duration.count() <= 3000); // 序列化应该在3秒内完成
}

// 测试大型数组的解析性能
TEST(Performance_ParseLargeArray) {
    const size_t arraySize = 10000000; // 1000万个元素
    
    std::cout << "Creating JSON array with " << arraySize << " elements..." << std::endl;
    std::vector<double> largeArray;
    largeArray.reserve(arraySize);
    
    for (size_t i = 0; i < arraySize; ++i) {
        largeArray.push_back(static_cast<double>(i) * 3.14159);
    }

    JsonValue json(largeArray);
    std::string jsonString = json.dump();
    
    // 清理内存
    largeArray.clear();
    json = JsonValue();
    
    // 记录开始时间
    auto start = std::chrono::high_resolution_clock::now();
    
    // 执行解析
    JsonValue parsed = JsonValue::parse(jsonString);
    
    // 记录结束时间
    auto end = std::chrono::high_resolution_clock::now();
    
    // 计算耗时
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Parsed array with " << arraySize << " elements in " << duration.count() << " ms" << std::endl;
    
    // 验证解析成功
    ASSERT_TRUE(parsed.isArray());
    ASSERT_EQ(parsed.size(), arraySize);
    
    // 性能断言（根据实际情况调整阈值）
    ASSERT_TRUE(duration.count() <= 3000); // 解析应该在3秒内完成
}

// 测试复杂嵌套结构的性能
TEST(Performance_ComplexNestedStructure) {
    const size_t outerSize = 1000;  // 外层1000个元素
    const size_t innerSize = 1000;  // 内层每个1000个元素
    
    std::cout << "Creating complex nested structure..." << std::endl;
    
    // 创建复杂嵌套结构
    std::map<std::string, std::vector<std::map<std::string, std::vector<int>>>> complexData;
    
    for (size_t i = 0; i < outerSize; ++i) {
        std::vector<std::map<std::string, std::vector<int>>> outerVec;
        outerVec.reserve(innerSize);
        
        for (size_t j = 0; j < innerSize; ++j) {
            std::map<std::string, std::vector<int>> innerMap;
            
            // 创建一个包含多个数组的map
            std::vector<int> array1, array2, array3;
            array1.reserve(10);
            array2.reserve(10);
            array3.reserve(10);
            
            for (int k = 0; k < 10; ++k) {
                array1.push_back(static_cast<int>(i * j + k));
                array2.push_back(static_cast<int>(i * j - k));
                array3.push_back(static_cast<int>(i + j + k));
            }
            
            innerMap["array1"] = array1;
            innerMap["array2"] = array2;
            innerMap["array3"] = array3;
            
            outerVec.push_back(innerMap);
        }
        
        complexData["group_" + std::to_string(i)] = outerVec;
    }
    
    // 记录开始时间
    auto start = std::chrono::high_resolution_clock::now();
    
    // 执行序列化
    JsonValue json(complexData);
    
    // 记录序列化结束时间
    auto serializeEnd = std::chrono::high_resolution_clock::now();
    
    // 获取JSON字符串大小
    std::string jsonString = json.dump();
    size_t jsonSize = jsonString.size();
    
    auto parseStart = std::chrono::high_resolution_clock::now();
    // 执行解析
    JsonValue parsed = JsonValue::parse(jsonString);
    
    // 记录解析结束时间
    auto parseEnd = std::chrono::high_resolution_clock::now();
    
    // 计算耗时
    auto serializeDuration = std::chrono::duration_cast<std::chrono::milliseconds>(serializeEnd - start);
    auto parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(parseEnd - parseStart);
    
    std::cout << "Complex nested structure serialization: " << serializeDuration.count() << " ms" << std::endl;
    std::cout << "Complex nested structure parsing: " << parseDuration.count() << " ms" << std::endl;
    std::cout << "Generated JSON size: " << jsonSize << " bytes" << std::endl;
    
    // 验证操作成功
    ASSERT_TRUE(json.isObject());
    ASSERT_TRUE(parsed.isObject());
    
    // 性能断言（根据实际情况调整阈值）
    ASSERT_TRUE(serializeDuration.count() <= 5000); // 序列化应该在5秒内完成
    ASSERT_TRUE(parseDuration.count() <= 5000);     // 解析应该在5秒内完成
}

int main() {
    std::cout << "=== Performance Limits Tests ===" << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    if (result == 0) {
        std::cout << "✅ All performance limits tests PASSED!" << std::endl;
    } else {
        std::cout << "❌ Some performance limits tests FAILED!" << std::endl;
    }
    
    return result;
}