#pragma once
#include "../type_registry/registry_core.h"
#include <vector>
#include <list>
#include <map>
#include <string>

namespace JsonStruct {

// Standard C++ type registration
class StdTypesRegistration {
public:
    static void registerAllStdTypes() {
        registerStdVector();
        registerStdList();
        registerStdMap();
    }

private:
    // std::vector<int> registration
    static void registerStdVector() {
        JsonStruct::TypeRegistry::instance().registerType<std::vector<int>>(
            // toJson
            [](const std::vector<int>& vec) -> JsonValue {
                JsonValue::ArrayType arr;
                for (const auto& item : vec) {
                    arr.push_back(JsonValue(item));
                }
                return JsonValue(arr);
            },
            // fromJson
            [](const JsonValue& json, const std::vector<int>& defaultValue) -> std::vector<int> {
                if (json.isArray()) {
                    std::vector<int> result;
                    const auto& arr = json.toArray();
                    for (size_t i = 0; i < arr.size(); ++i) {
                        result.push_back(arr[i].toInt(0));
                    }
                    return result;
                }
                return defaultValue;
            }
        );

        // std::vector<std::string> registration
        JsonStruct::TypeRegistry::instance().registerType<std::vector<std::string>>(
            // toJson
            [](const std::vector<std::string>& vec) -> JsonValue {
                JsonValue::ArrayType arr;
                for (const auto& item : vec) {
                    arr.push_back(JsonValue(item));
                }
                return JsonValue(arr);
            },
            // fromJson
            [](const JsonValue& json, const std::vector<std::string>& defaultValue) -> std::vector<std::string> {
                if (json.isArray()) {
                    std::vector<std::string> result;
                    const auto& arr = json.toArray();
                    for (size_t i = 0; i < arr.size(); ++i) {
                        result.push_back(arr[i].toString(""));
                    }
                    return result;
                }
                return defaultValue;
            }
        );

        // std::vector<std::vector<int>> registration (nested vector)
        JsonStruct::TypeRegistry::instance().registerType<std::vector<std::vector<int>>>(
            // toJson
            [](const std::vector<std::vector<int>>& vec) -> JsonValue {
                JsonValue::ArrayType arr;
                for (const auto& innerVec : vec) {
                    JsonValue::ArrayType innerArr;
                    for (const auto& item : innerVec) {
                        innerArr.push_back(JsonValue(item));
                    }
                    arr.push_back(JsonValue(innerArr));
                }
                return JsonValue(arr);
            },
            // fromJson
            [](const JsonValue& json, const std::vector<std::vector<int>>& defaultValue) -> std::vector<std::vector<int>> {
                if (json.isArray()) {
                    std::vector<std::vector<int>> result;
                    const auto& arr = json.toArray();
                    for (size_t i = 0; i < arr.size(); ++i) {
                        if (arr[i].isArray()) {
                            std::vector<int> innerVec;
                            const auto& innerArr = arr[i].toArray();
                            for (size_t j = 0; j < innerArr.size(); ++j) {
                                innerVec.push_back(innerArr[j].toInt(0));
                            }
                            result.push_back(innerVec);
                        }
                    }
                    return result;
                }
                return defaultValue;
            }
        );
    }
    
    // std::list<int> registration
    static void registerStdList() {
        JsonStruct::TypeRegistry::instance().registerType<std::list<int>>(
            // toJson
            [](const std::list<int>& lst) -> JsonValue {
                JsonValue::ArrayType arr;
                for (const auto& item : lst) {
                    arr.push_back(JsonValue(item));
                }
                return JsonValue(arr);
            },
            // fromJson
            [](const JsonValue& json, const std::list<int>& defaultValue) -> std::list<int> {
                if (json.isArray()) {
                    std::list<int> result;
                    const auto& arr = json.toArray();
                    for (size_t i = 0; i < arr.size(); ++i) {
                        result.push_back(arr[i].toInt(0));
                    }
                    return result;
                }
                return defaultValue;
            }
        );
    }
    
    // std::map<std::string, int> registration
    static void registerStdMap() {
        JsonStruct::TypeRegistry::instance().registerType<std::map<std::string, int>>(
            // toJson
            [](const std::map<std::string, int>& mp) -> JsonValue {
                JsonValue::ObjectType obj;
                for (auto it = mp.begin(); it != mp.end(); ++it) {
                    obj[it->first] = JsonValue(it->second);
                }
                return JsonValue(obj);
            },
            // fromJson
            [](const JsonValue& json, const std::map<std::string, int>& defaultValue) -> std::map<std::string, int> {
                if (json.isObject()) {
                    std::map<std::string, int> result;
                    const auto& obj = json.toObject();
                    for (auto it = obj.begin(); it != obj.end(); ++it) {
                        result[it->first] = it->second.toInt(0);
                    }
                    return result;
                }
                return defaultValue;
            }
        );

        // std::map<std::string, std::string> registration
        JsonStruct::TypeRegistry::instance().registerType<std::map<std::string, std::string>>(
            // toJson
            [](const std::map<std::string, std::string>& mp) -> JsonValue {
                JsonValue::ObjectType obj;
                for (auto it = mp.begin(); it != mp.end(); ++it) {
                    obj[it->first] = JsonValue(it->second);
                }
                return JsonValue(obj);
            },
            // fromJson
            [](const JsonValue& json, const std::map<std::string, std::string>& defaultValue) -> std::map<std::string, std::string> {
                if (json.isObject()) {
                    std::map<std::string, std::string> result;
                    const auto& obj = json.toObject();
                    for (auto it = obj.begin(); it != obj.end(); ++it) {
                        result[it->first] = it->second.toString("");
                    }
                    return result;
                }
                return defaultValue;
            }
        );
    }
};

// Automatic registration static initializer for standard types
namespace {
    struct StdTypesAutoRegistrar {
        StdTypesAutoRegistrar() {
            StdTypesRegistration::registerAllStdTypes();
        }
    };
    static StdTypesAutoRegistrar std_types_auto_registrar;
}

} // namespace JsonStruct
