// 迁移的自定义类型测试 - Custom Types Tests
#include "../src/jsonstruct.h"
#include "test_framework.h"
#include <iostream>
#include <cmath>

using namespace JsonStruct;

// Example custom type: 3D point
struct Point3D {
    double x, y, z;
    Point3D(double x = 0., double y = 0., double z = 0.) : x(x), y(y), z(z) {}

    bool operator==(const Point3D& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct UserInfo {
    std::string name;
    int age;
    std::vector<std::string> hobbies;
    
    UserInfo() : age(0) {}
    UserInfo(const std::string& name, int age) : name(name), age(age) {}
    
    bool operator==(const UserInfo& other) const {
        return name == other.name && age == other.age && hobbies == other.hobbies;
    }
};

struct ComplexData {
    Point3D position;
    UserInfo user;
    std::vector<Point3D> path;
    
    ComplexData() = default;
    
    bool operator==(const ComplexData& other) const {
        return position == other.position && 
               user == other.user && 
               path == other.path;
    }
};

// Helper function to register types
void registerCustomTypes() {
    // Register Point3D type
    TypeRegistry::instance().registerType<Point3D>(
        // Serialization function
        [](const Point3D& point) -> JsonValue {
            JsonValue::ArrayType arr;
            arr.push_back(JsonValue(point.x));
            arr.push_back(JsonValue(point.y));
            arr.push_back(JsonValue(point.z));
            return JsonValue(arr);
        },
        // Deserialization function
        [](const JsonValue& json, const Point3D& defaultValue) -> Point3D {
            if (const auto& arrOpt = json.toArray()) {
                const auto& arr = arrOpt->get();
                if (arr.size() == 3) {
                    return Point3D(arr[0].toDouble(), arr[1].toDouble(), arr[2].toDouble());
                }
            }
            return defaultValue;
        }
    );

    // Register UserInfo type
    TypeRegistry::instance().registerType<UserInfo>(
        // Serialization function
        [](const UserInfo& user) -> JsonValue {
            JsonValue::ObjectType obj;
            obj["name"] = JsonValue(user.name);
            obj["age"] = JsonValue(user.age);
            
            // 序列化hobbies数组
            JsonValue::ArrayType hobbiesArr;
            for (const auto& hobby : user.hobbies) {
                hobbiesArr.push_back(JsonValue(hobby));
            }
            obj["hobbies"] = JsonValue(hobbiesArr);
            
            return JsonValue(obj);
        },
        // Deserialization function
        [](const JsonValue& json, const UserInfo& defaultValue) -> UserInfo {
            if (const auto& objOpt = json.toObject()) {
                const auto& obj = objOpt->get();
                UserInfo user;
                
                auto nameIt = obj.find("name");
                if (nameIt != obj.end()) {
                    user.name = nameIt->second.toString();
                }
                
                auto ageIt = obj.find("age");
                if (ageIt != obj.end()) {
                    user.age = ageIt->second.toInt(0);
                }
                
                auto hobbiesIt = obj.find("hobbies");
                if (hobbiesIt != obj.end() && hobbiesIt->second.isArray()) {
                    if(const auto& hobbiesArr = hobbiesIt->second.toArray()) {
                        for (const auto& hobby : hobbiesArr->get()) {
                            user.hobbies.emplace_back(hobby.toString());
                        }
                    }
                }
                
                return user;
            }
            return defaultValue;
        }
    );

    // Register ComplexData type
    TypeRegistry::instance().registerType<ComplexData>(
        // Serialization function
        [](const ComplexData& data) -> JsonValue {
            JsonValue::ObjectType obj;
            
            // 使用已注册的Point3D序列化器
            obj["position"] = TypeRegistry::instance().toJson(data.position);
            
            // 使用已注册的UserInfo序列化器
            obj["user"] = TypeRegistry::instance().toJson(data.user);
            
            // 序列化Point3D数组
            JsonValue::ArrayType pathArr;
            for (const auto& point : data.path) {
                pathArr.push_back(TypeRegistry::instance().toJson(point));
            }
            obj["path"] = JsonValue(pathArr);
            
            return JsonValue(obj);
        },
        // Deserialization function
        [](const JsonValue& json, const ComplexData& defaultValue) -> ComplexData {
            if (const auto& objOpt = json.toObject()) {
                const auto& obj = objOpt->get();
                ComplexData data;
                
                auto posIt = obj.find("position");
                if (posIt != obj.end()) {
                    data.position = TypeRegistry::instance().fromJson<Point3D>(posIt->second, Point3D());
                }
                
                auto userIt = obj.find("user");
                if (userIt != obj.end()) {
                    data.user = TypeRegistry::instance().fromJson<UserInfo>(userIt->second, UserInfo());
                }
                
                auto pathIt = obj.find("path");
                if (pathIt != obj.end() && pathIt->second.isArray()) {
                    if(const auto& pathArrOpt = pathIt->second.toArray()) {
                        const auto & pathArr = pathArrOpt->get();
                        for (const auto& item : pathArr) {
                            data.path.push_back(TypeRegistry::instance().fromJson<Point3D>(item, Point3D()));
                        }
                    }
                }
                
                return data;
            }
            return defaultValue;
        }
    );
}

TEST(CustomTypes_Point3D) {
    registerCustomTypes();
    
    Point3D original(1.0, 2.0, 3.0);
    
    // Test serialization
    JsonValue json = TypeRegistry::instance().toJson(original);
    ASSERT_TRUE(json.isArray());
    ASSERT_EQ(json.size(), 3);
    ASSERT_NEAR(json[0].toDouble(), 1.0, 0.001);
    ASSERT_NEAR(json[1].toDouble(), 2.0, 0.001);
    ASSERT_NEAR(json[2].toDouble(), 3.0, 0.001);
    
    // Test deserialization
    Point3D restored = TypeRegistry::instance().fromJson<Point3D>(json, Point3D());
    ASSERT_TRUE(original == restored);
}

TEST(CustomTypes_UserInfo) {
    registerCustomTypes();
    
    UserInfo original("Alice", 25);
    original.hobbies = {"reading", "coding", "music"};
    
    // Test serialization
    JsonValue json = TypeRegistry::instance().toJson(original);
    ASSERT_TRUE(json.isObject());
    ASSERT_EQ(json["name"].toString(), "Alice");
    ASSERT_EQ(json["age"].toInt(), 25);
    ASSERT_TRUE(json["hobbies"].isArray());
    ASSERT_EQ(json["hobbies"].size(), 3);
    
    // Test deserialization
    UserInfo restored = TypeRegistry::instance().fromJson<UserInfo>(json, UserInfo());
    ASSERT_TRUE(original == restored);
    ASSERT_EQ(restored.hobbies.size(), 3);
    ASSERT_EQ(restored.hobbies[0], "reading");
    ASSERT_EQ(restored.hobbies[1], "coding");
    ASSERT_EQ(restored.hobbies[2], "music");
}

TEST(CustomTypes_ComplexData) {
    registerCustomTypes();
    
    Point3D point(1.0, 2.0, 3.0);
    UserInfo user("Alice", 25);
    user.hobbies = {"reading", "coding", "music"};
    
    ComplexData original;
    original.position = point;
    original.user = user;
    original.path = {Point3D(0,0,0), Point3D(1,1,1), Point3D(2,2,2)};
    
    // Test serialization
    JsonValue json = TypeRegistry::instance().toJson(original);
    ASSERT_TRUE(json.isObject());
    ASSERT_TRUE(json["position"].isArray());
    ASSERT_TRUE(json["user"].isObject());
    ASSERT_TRUE(json["path"].isArray());
    ASSERT_EQ(json["path"].size(), 3);
    
    // Test JSON string roundtrip
    std::string jsonStr = json.dump(2);
    ASSERT_FALSE(jsonStr.empty());
    
    JsonValue parsed = JsonValue::parse(jsonStr);
    ComplexData restored = TypeRegistry::instance().fromJson<ComplexData>(parsed, ComplexData());
    
    // Verify all data is preserved
    // Verify all data is preserved through comparison
    ASSERT_TRUE(original == restored);
    ASSERT_EQ(restored.user.name, "Alice");
    ASSERT_EQ(restored.user.age, 25);
    ASSERT_EQ(restored.user.hobbies.size(), 3);
    ASSERT_EQ(restored.path.size(), 3);
    
    // Verify specific path points
    ASSERT_EQ(restored.path[0].x, 0.0);
    ASSERT_EQ(restored.path[1].x, 1.0);
    ASSERT_EQ(restored.path[2].x, 2.0);
}

TEST(CustomTypes_ErrorHandling) {
    registerCustomTypes();
    
    // Test invalid JSON for Point3D
    JsonValue invalidPoint = JsonValue("not an array");
    Point3D defaultPoint(99, 99, 99);
    Point3D pointResult = TypeRegistry::instance().fromJson<Point3D>(invalidPoint, defaultPoint);
    ASSERT_TRUE(pointResult == defaultPoint);
    
    // Test incomplete array for Point3D
    JsonValue::ArrayType incompleteArr;
    incompleteArr.push_back(JsonValue(1.0));
    incompleteArr.push_back(JsonValue(2.0));
    // Missing third element
    JsonValue incompleteJson(incompleteArr);
    Point3D incompleteResult = TypeRegistry::instance().fromJson<Point3D>(incompleteJson, defaultPoint);
    ASSERT_TRUE(incompleteResult == defaultPoint);
    
    // Test invalid JSON for UserInfo
    JsonValue invalidUser = JsonValue("not an object");
    UserInfo defaultUser("default", -1);
    UserInfo userResult = TypeRegistry::instance().fromJson<UserInfo>(invalidUser, defaultUser);
    ASSERT_TRUE(userResult == defaultUser);
}

int main() {
    std::cout << "=== Custom Types Migration Tests ===" << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    if (result == 0) {
        std::cout << "✅ All custom types tests PASSED!" << std::endl;
    } else {
        std::cout << "❌ Some custom types tests FAILED!" << std::endl;
    }
    
    return result;
}
