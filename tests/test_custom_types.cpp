#include "jsonstruct.h"
#include <cmath>
#include <iostream>
#include <cassert>

namespace JsonStruct {

// Example custom type: 3D point
struct Point3D {
    double x, y, z;
    Point3D(double x = 0., double y=0., double z=0.) : x(x), y(y), z(z) {}

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

// Register Point3D type
REGISTER_JSON_TYPE(Point3D,
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
        if (json.isArray()) {
            const auto& arr = json.toArray();
            if (arr.size() == 3) {
                return Point3D(arr[0].toDouble(), arr[1].toDouble(), arr[2].toDouble());
            }
        }
        return defaultValue;
    }
);

// Register UserInfo type
REGISTER_JSON_TYPE(UserInfo,
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
    // Deserialization function - 修复版本
    [](const JsonValue& json, const UserInfo& defaultValue) -> UserInfo {
        if (json.isObject()) {
            const auto& obj = json.toObject();
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
                const auto& hobbiesArr = hobbiesIt->second.toArray();
                for (size_t i = 0; i < hobbiesArr.size(); ++i) {
                    user.hobbies.emplace_back(hobbiesArr[i].toString());
                }
            }
            
            return user;
        }
        return defaultValue;
    }
);

// 示例复合类型：包含其他注册类型的结构
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

// Register ComplexData type, demonstrating how to combine registered types
REGISTER_JSON_TYPE(ComplexData,
    // Serialization function
    [](const ComplexData& data) -> JsonValue {
        JsonValue::ObjectType obj;
        
        // 使用已注册的Point3D序列化器
        obj["position"] = JsonStruct::TypeRegistry::instance().toJson(data.position);
        
        // 使用已注册的UserInfo序列化器
        obj["user"] = JsonStruct::TypeRegistry::instance().toJson(data.user);
        
        // 序列化Point3D数组
        JsonValue::ArrayType pathArr;
        for (const auto& point : data.path) {
            pathArr.push_back(JsonStruct::TypeRegistry::instance().toJson(point));
        }
        obj["path"] = JsonValue(pathArr);
        
        return JsonValue(obj);
    },
    // Deserialization function
    [](const JsonValue& json, const ComplexData& defaultValue) -> ComplexData {
        if (json.isObject()) {
            const auto& obj = json.toObject();
            ComplexData data;
            
            auto posIt = obj.find("position");
            if (posIt != obj.end()) {
                data.position = JsonStruct::TypeRegistry::instance().fromJson<Point3D>(posIt->second, Point3D());
            }
            
            auto userIt = obj.find("user");
            if (userIt != obj.end()) {
                data.user = JsonStruct::TypeRegistry::instance().fromJson<UserInfo>(userIt->second, UserInfo());
            }
            
            auto pathIt = obj.find("path");
            if (pathIt != obj.end() && pathIt->second.isArray()) {
                const auto& pathArr = pathIt->second.toArray();
                for (size_t i = 0; i < pathArr.size(); ++i) {
                    data.path.push_back(JsonStruct::TypeRegistry::instance().fromJson<Point3D>(pathArr[i], Point3D()));
                }
            }
            
            return data;
        }
        return defaultValue;
    }
);

} // namespace JsonStruct

int main() {
    using namespace JsonStruct;
    
    Point3D point(1.0, 2.0, 3.0);
    UserInfo user("Alice", 25);
    user.hobbies = {"reading", "coding", "music"};
    
    ComplexData complex;
    complex.position = point;
    complex.user = user;
    complex.path = {Point3D(0,0,0), Point3D(1,1,1), Point3D(2,2,2)};
    
    JsonValue json = TypeRegistry::instance().toJson(complex);
    std::string jsonStr = json.dump(2);
    std::cout << "Serialized: " << jsonStr << std::endl;
    
    JsonValue parsed = JsonValue::parse(jsonStr);
    ComplexData restored = TypeRegistry::instance().fromJson<ComplexData>(parsed, ComplexData());
    
    std::cout << "\n=== Test Results ===" << std::endl;
    std::cout << "Original user name: '" << complex.user.name << "'" << std::endl;
    std::cout << "Restored user name: '" << restored.user.name << "'" << std::endl;
    std::cout << "Original user hobbies count: " << complex.user.hobbies.size() << std::endl;
    std::cout << "Restored user hobbies count: " << restored.user.hobbies.size() << std::endl;
    if (!restored.user.hobbies.empty()) {
        std::cout << "First restored hobby: '" << restored.user.hobbies[0] << "'" << std::endl;
    }
    std::cout << "===================" << std::endl;
    
    assert(complex == restored);
    std::cout << "✅ All tests passed!" << std::endl;
    
    return 0;
}

