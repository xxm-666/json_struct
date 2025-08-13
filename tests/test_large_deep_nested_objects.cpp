#include <test_framework/test_framework.h>
#include "../src/type_registry/registry_core.h"
#include "../src/type_registry/auto_serializer.h"
#include "../src/json_engine/json_value.h"
#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <utility>
#include <limits>

using namespace JsonStruct;

// 测试超大/超深嵌套对象与数组
struct LargeDeepNestedTest {
    JSON_AUTO(deeplyNestedMap, largeVector, complexNestedStructure)
    
    // 深度嵌套的map
    std::map<std::string, std::map<std::string, std::map<std::string, std::map<std::string, int>>>> deeplyNestedMap;
    
    // 大型vector
    std::vector<std::vector<std::vector<int>>> largeVector;
    
    // 复杂嵌套结构
    std::map<std::string, std::vector<std::map<std::string, std::pair<std::vector<int>, std::map<int, std::string>>>>> complexNestedStructure;
};

TEST(LargeDeepNested_ExtremeDeepNesting) {
    LargeDeepNestedTest obj;
    
    // 创建深度嵌套的map
    // 4层嵌套深度
    obj.deeplyNestedMap["level1"]["level2"]["level3"]["level4"] = 12345;
    
    // 创建大型vector
    // 3层嵌套，每层100个元素
    obj.largeVector.resize(100);
    for (int i = 0; i < 100; ++i) {
        obj.largeVector[i].resize(100);
        for (int j = 0; j < 100; ++j) {
            obj.largeVector[i][j].resize(100);
            for (int k = 0; k < 100; ++k) {
                obj.largeVector[i][j][k] = i * 10000 + j * 100 + k;
            }
        }
    }
    
    // 序列化
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    // 反序列化
    LargeDeepNestedTest restored;
    fromJson(restored, json);
    
    // 验证数据
    ASSERT_EQ(obj.deeplyNestedMap.size(), restored.deeplyNestedMap.size());
    ASSERT_EQ(obj.deeplyNestedMap["level1"]["level2"]["level3"]["level4"], 
              restored.deeplyNestedMap["level1"]["level2"]["level3"]["level4"]);
    
    ASSERT_EQ(obj.largeVector.size(), restored.largeVector.size());
    ASSERT_EQ(obj.largeVector[50][50].size(), restored.largeVector[50][50].size());
    ASSERT_EQ(obj.largeVector[50][50][50], restored.largeVector[50][50][50]);
}

TEST(LargeDeepNested_ExtremeLargeArrays) {
    LargeDeepNestedTest obj;
    
    // 创建一个非常大的vector
    const size_t largeSize = 10000;  // 10K元素
    std::vector<int> largeArray;
    largeArray.reserve(largeSize);
    for (size_t i = 0; i < largeSize; ++i) {
        largeArray.push_back(static_cast<int>(i));
    }
    
    // 创建嵌套的大vector
    obj.largeVector.resize(10);  // 10个元素
    for (int i = 0; i < 10; ++i) {
        obj.largeVector[i].resize(10);  // 每个元素包含10个vector
        for (int j = 0; j < 10; ++j) {
            obj.largeVector[i][j] = largeArray;  // 每个内部vector包含10K个元素
        }
    }
    
    // 序列化
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    // 反序列化
    LargeDeepNestedTest restored;
    fromJson(restored, json);
    
    // 验证数据
    ASSERT_EQ(obj.largeVector.size(), restored.largeVector.size());
    ASSERT_EQ(obj.largeVector[5][5].size(), restored.largeVector[5][5].size());
    ASSERT_EQ(obj.largeVector[5][5][5000], restored.largeVector[5][5][5000]);
}

TEST(LargeDeepNested_ComplexNestedStructure) {
    LargeDeepNestedTest obj;
    
    // 创建复杂嵌套结构
    for (int i = 0; i < 100; ++i) {
        std::string key = "key_" + std::to_string(i);
        std::vector<std::map<std::string, std::pair<std::vector<int>, std::map<int, std::string>>>> vec;
        vec.resize(10);
        
        for (int j = 0; j < 10; ++j) {
            std::map<std::string, std::pair<std::vector<int>, std::map<int, std::string>>> innerMap;
            
            // 创建pair中的vector
            std::vector<int> intVec;
            intVec.reserve(100);
            for (int k = 0; k < 100; ++k) {
                intVec.push_back(i * 1000 + j * 100 + k);
            }
            
            // 创建pair中的map
            std::map<int, std::string> strMap;
            for (int k = 0; k < 50; ++k) {
                strMap[k] = "value_" + std::to_string(i) + "_" + std::to_string(j) + "_" + std::to_string(k);
            }
            
            innerMap["inner_key_" + std::to_string(j)] = std::make_pair(intVec, strMap);
            vec[j] = innerMap;
        }
        
        obj.complexNestedStructure[key] = vec;
    }
    
    // 序列化
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    // 反序列化
    LargeDeepNestedTest restored;
    fromJson(restored, json);
    
    // 验证数据
    ASSERT_EQ(obj.complexNestedStructure.size(), restored.complexNestedStructure.size());
    
    // 验证部分数据
    auto it = obj.complexNestedStructure.find("key_50");
    if (it != obj.complexNestedStructure.end()) {
        ASSERT_EQ(it->second.size(), restored.complexNestedStructure["key_50"].size());
        ASSERT_EQ(it->second[5]["inner_key_5"].first.size(), 
                  restored.complexNestedStructure["key_50"][5]["inner_key_5"].first.size());
        ASSERT_EQ(it->second[5]["inner_key_5"].first[50], 
                  restored.complexNestedStructure["key_50"][5]["inner_key_5"].first[50]);
    }
}

TEST(LargeDeepNested_TypeConversion) {
    LargeDeepNestedTest obj;
    
    // 创建包含各种数据类型的复杂结构
    // 深度嵌套map
    obj.deeplyNestedMap["int"]["value"]["test"]["1"] = 12345;
    obj.deeplyNestedMap["int"]["value"]["test"]["2"] = -98765;
    
    // 大型vector包含不同类型
    obj.largeVector.resize(3);
    obj.largeVector[0].resize(1);
    obj.largeVector[0][0] = {std::numeric_limits<int>::max(), std::numeric_limits<int>::min(), 0};
    
    obj.largeVector[1].resize(1);
    obj.largeVector[1][0] = {1, 2, 3};
    
    obj.largeVector[2].resize(1);
    obj.largeVector[2][0] = {-1, -2, -3};
    
    // 序列化
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    // 反序列化
    LargeDeepNestedTest restored;
    fromJson(restored, json);
    
    // 验证类型转换
    ASSERT_EQ(obj.deeplyNestedMap["int"]["value"]["test"]["1"], 
              restored.deeplyNestedMap["int"]["value"]["test"]["1"]);
    ASSERT_EQ(obj.deeplyNestedMap["int"]["value"]["test"]["2"], 
              restored.deeplyNestedMap["int"]["value"]["test"]["2"]);
    
    ASSERT_EQ(obj.largeVector[0][0][0], restored.largeVector[0][0][0]);
    ASSERT_EQ(obj.largeVector[0][0][1], restored.largeVector[0][0][1]);
    ASSERT_EQ(obj.largeVector[0][0][2], restored.largeVector[0][0][2]);
}

int main() {
    std::cout << "=== Large and Deep Nested Objects Tests ===" << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    if (result == 0) {
        std::cout << "✅ All large and deep nested objects tests PASSED!" << std::endl;
    } else {
        std::cout << "❌ Some large and deep nested objects tests FAILED!" << std::endl;
    }
    
    return result;
}