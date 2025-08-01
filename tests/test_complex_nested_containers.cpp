#include "../test_framework/test_framework.h"
#include "../src/type_registry/registry_core.h"
#include "../src/type_registry/auto_serializer.h"
#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <utility>

using namespace JsonStruct;

// 测试复杂嵌套容器类型
struct ComplexContainerTest {
    JSON_AUTO(vectorOfVectorOfPairs, mapOfVectorOfSets, listOfMaps)
    
    // vector<vector<pair<int, string>>>
    std::vector<std::vector<std::pair<int, std::string>>> vectorOfVectorOfPairs;
    
    // map<string, vector<set<int>>>
    std::map<std::string, std::vector<std::set<int>>> mapOfVectorOfSets;
    
    // list<map<string, pair<double, bool>>>
    std::list<std::map<std::string, std::pair<double, bool>>> listOfMaps;
};

TEST(ComplexContainer_VectorOfVectorOfPairs) {
    ComplexContainerTest obj;
    
    // 创建测试数据
    obj.vectorOfVectorOfPairs = {
        {{1, "a"}, {2, "b"}},
        {{3, "c"}, {4, "d"}, {5, "e"}}
    };
    
    // 序列化
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    // 反序列化
    ComplexContainerTest restored;
    fromJson(restored, json);
    
    // 验证数据
    ASSERT_EQ(obj.vectorOfVectorOfPairs.size(), restored.vectorOfVectorOfPairs.size());
    ASSERT_EQ(obj.vectorOfVectorOfPairs[0].size(), restored.vectorOfVectorOfPairs[0].size());
    ASSERT_EQ(obj.vectorOfVectorOfPairs[0][0].first, restored.vectorOfVectorOfPairs[0][0].first);
    ASSERT_EQ(obj.vectorOfVectorOfPairs[0][0].second, restored.vectorOfVectorOfPairs[0][0].second);
    ASSERT_EQ(obj.vectorOfVectorOfPairs[1][2].first, restored.vectorOfVectorOfPairs[1][2].first);
    ASSERT_EQ(obj.vectorOfVectorOfPairs[1][2].second, restored.vectorOfVectorOfPairs[1][2].second);
}

TEST(ComplexContainer_MapOfVectorOfSets) {
    ComplexContainerTest obj;
    
    // 创建测试数据
    obj.mapOfVectorOfSets["first"] = {
        {1, 2, 3},
        {4, 5}
    };
    obj.mapOfVectorOfSets["second"] = {
        {10, 20}
    };
    
    // 序列化
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    // 反序列化
    ComplexContainerTest restored;
    fromJson(restored, json);
    
    // 验证数据
    ASSERT_EQ(obj.mapOfVectorOfSets.size(), restored.mapOfVectorOfSets.size());
    ASSERT_EQ(obj.mapOfVectorOfSets["first"].size(), restored.mapOfVectorOfSets["first"].size());
    
    // 验证set中的元素（set会自动排序）
    auto& originalSet = obj.mapOfVectorOfSets["first"][0];
    auto& restoredSet = restored.mapOfVectorOfSets["first"][0];
    ASSERT_EQ(originalSet.size(), restoredSet.size());
    
    // 验证set包含相同的元素
    for (int val : originalSet) {
        ASSERT_TRUE(restoredSet.find(val) != restoredSet.end());
    }
}

TEST(ComplexContainer_ListOfMaps) {
    ComplexContainerTest obj;
    
    // 创建测试数据
    std::map<std::string, std::pair<double, bool>> map1;
    map1["pi"] = {3.14159, true};
    map1["e"] = {2.71828, false};
    
    std::map<std::string, std::pair<double, bool>> map2;
    map2["sqrt2"] = {1.41421, true};
    
    obj.listOfMaps.push_back(map1);
    obj.listOfMaps.push_back(map2);
    
    // 序列化
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    // 反序列化
    ComplexContainerTest restored;
    fromJson(restored, json);
    
    // 验证数据
    ASSERT_EQ(obj.listOfMaps.size(), restored.listOfMaps.size());
    
    auto it1 = obj.listOfMaps.begin();
    auto it2 = restored.listOfMaps.begin();
    
    // 验证第一个map
    ASSERT_EQ(it1->size(), it2->size());
    ASSERT_NEAR((*it1)["pi"].first, (*it2)["pi"].first, 0.00001);
    ASSERT_EQ((*it1)["pi"].second, (*it2)["pi"].second);
    ASSERT_NEAR((*it1)["e"].first, (*it2)["e"].first, 0.00001);
    ASSERT_EQ((*it1)["e"].second, (*it2)["e"].second);
    
    // 验证第二个map
    ++it1; ++it2;
    ASSERT_EQ(it1->size(), it2->size());
    ASSERT_NEAR((*it1)["sqrt2"].first, (*it2)["sqrt2"].first, 0.00001);
    ASSERT_EQ((*it1)["sqrt2"].second, (*it2)["sqrt2"].second);
}

TEST(ComplexContainer_EmptyContainers) {
    ComplexContainerTest obj;
    // 保持所有容器为空
    
    // 序列化
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    // 反序列化
    ComplexContainerTest restored;
    fromJson(restored, json);
    
    // 验证空容器
    ASSERT_TRUE(restored.vectorOfVectorOfPairs.empty());
    ASSERT_TRUE(restored.mapOfVectorOfSets.empty());
    ASSERT_TRUE(restored.listOfMaps.empty());
}

TEST(ComplexContainer_EdgeCases) {
    ComplexContainerTest obj;
    
    // 创建一些边界情况的数据
    obj.vectorOfVectorOfPairs = {
        {}, // 空的内层vector
        {{0, ""}}, // 包含默认值的pair
        {{-1, "negative"}, {999999, "large"}}
    };
    
    obj.mapOfVectorOfSets["empty"] = {}; // 空vector
    obj.mapOfVectorOfSets["withEmptySet"] = {{}}; // 包含空set的vector
    
    // 序列化
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    // 反序列化
    ComplexContainerTest restored;
    fromJson(restored, json);
    
    // 验证边界情况
    ASSERT_EQ(obj.vectorOfVectorOfPairs.size(), restored.vectorOfVectorOfPairs.size());
    ASSERT_TRUE(restored.vectorOfVectorOfPairs[0].empty()); // 空的内层vector
    ASSERT_EQ(obj.vectorOfVectorOfPairs[1][0].first, restored.vectorOfVectorOfPairs[1][0].first);
    ASSERT_EQ(obj.vectorOfVectorOfPairs[1][0].second, restored.vectorOfVectorOfPairs[1][0].second);
    
    ASSERT_TRUE(restored.mapOfVectorOfSets["empty"].empty());
    ASSERT_EQ(restored.mapOfVectorOfSets["withEmptySet"].size(), 1);
    ASSERT_TRUE(restored.mapOfVectorOfSets["withEmptySet"][0].empty());
}

int main() {   
    return RUN_ALL_TESTS();
}
