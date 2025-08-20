#include "jsonstruct.h"
#include <string>
#include <cstdint>
#include <iostream>
#include <vector>
#include <memory>

using namespace JsonStruct;

// 测试用的复杂结构体
struct NestedObject {
    JSON_AUTO(id, data, metadata)
    int id = 0;
    std::string data;
    std::vector<std::string> metadata;
};

struct ComplexStructure {
    JSON_AUTO(name, value, flag, nested, items, optional_field)
    std::string name;
    double value = 0.0;
    bool flag = false;
    NestedObject nested;
    std::vector<NestedObject> items;
    std::string optional_field;
};

// 边界测试用的结构体
struct EdgeCaseStruct {
    JSON_AUTO(huge_number, tiny_number, special_chars, unicode_text, empty_arrays, null_values)
    long long huge_number = 0;
    double tiny_number = 0.0;
    std::string special_chars;
    std::string unicode_text;
    std::vector<int> empty_arrays;
    std::string null_values;
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size == 0) {
        return 0;
    }

    std::string json_string(reinterpret_cast<const char*>(Data), Size);
    
    // 测试1: 基础 JSON 解析
    try {
        JsonValue value(json_string);
        
        // 尝试访问不同类型的值
        if (value.isObject()) {
            auto* obj = value.getObject();
            if (obj) {
                // 遍历对象的键值对
                for (const auto& pair : *obj) {
                    if (pair.second.isString()) {
                        auto str = pair.second.getString();
                    } else if (pair.second.isNumber()) {
                        auto num = pair.second.getNumber();
                    } else if (pair.second.isBool()) {
                        auto flag = pair.second.getBool();
                    } else if (pair.second.isArray()) {
                        auto* arr = pair.second.getArray();
                        if (arr) {
                            for (const auto& item : *arr) {
                                // 递归测试数组元素
                                if (item.isString()) {
                                    auto str_item = item.getString();
                                }
                            }
                        }
                    }
                }
            }
        } else if (value.isArray()) {
            auto* arr = value.getArray();
            if (arr) {
                for (const auto& item : *arr) {
                    if (item.isObject()) {
                        auto* obj_item = item.getObject();
                    }
                }
            }
        }
    } catch (...) {
        // 捕获解析异常
    }

    // 测试2: 复杂结构体解析
    try {
        ComplexStructure complex;
        JsonValue json_val(json_string);
        fromJson(complex, json_val);
        
        // 尝试序列化回去
        JsonValue serialized = toJson(complex);
        std::string serialized_str = serialized.toString();
        
        // 再次解析序列化后的结果
        ComplexStructure complex2;
        fromJson(complex2, serialized);
        
    } catch (...) {
        // 捕获序列化/反序列化异常
    }

    // 测试3: 边界情况结构体
    try {
        EdgeCaseStruct edge_case;
        JsonValue json_val(json_string);
        fromJson(edge_case, json_val);
        
        // 测试极端值
        if (edge_case.huge_number > 0 && edge_case.huge_number < LLONG_MAX) {
            // 验证大数字处理
        }
        
        if (edge_case.tiny_number > -1.0 && edge_case.tiny_number < 1.0) {
            // 验证小数字处理
        }
        
        if (!edge_case.special_chars.empty()) {
            // 验证特殊字符处理
        }
        
    } catch (...) {
        // 捕获边界情况异常
    }

    // 测试4: 基本的JSON结构验证（代替JSON Path，因为API不可用）
    try {
        JsonValue value(json_string);
        if (value.isObject()) {
            auto* obj = value.getObject();
            if (obj) {
                // 检查常见字段
                for (const auto& pair : *obj) {
                    const std::string& key = pair.first;
                    const JsonValue& val = pair.second;
                    
                    // 测试各种值类型的访问
                    if (key == "name" && val.isString()) {
                        auto name = val.getString();
                    } else if (key == "id" && val.isNumber()) {
                        auto id = val.getNumber();
                    } else if (key == "items" && val.isArray()) {
                        auto* items = val.getArray();
                        if (items && items->size() > 0) {
                            for (const auto& item : *items) {
                                if (item.isObject()) {
                                    auto* item_obj = item.getObject();
                                }
                            }
                        }
                    }
                }
            }
        }
    } catch (...) {
        // 捕获JSON结构验证异常
    }

    // 测试5: JSON结构克隆和比较（代替JSON Patch）
    try {
        JsonValue original(json_string);
        if (original.isObject()) {
            // 创建副本进行修改测试
            JsonValue copy = original;
            
            // 测试不同的修改操作（通过重新构造）
            if (auto* obj = original.getObject()) {
                // 测试对象的基本操作
                size_t size = obj->size();
                bool empty = obj->empty();
                
                // 检查特定键是否存在
                auto it = obj->find("name");
                if (it != obj->end()) {
                    const JsonValue& name_value = it->second;
                }
                
                // 测试迭代器稳定性
                for (auto it = obj->begin(); it != obj->end(); ++it) {
                    const std::string& key = it->first;
                    const JsonValue& value = it->second;
                    
                    // 基本类型检查
                    if (value.isString() || value.isNumber() || value.isBool()) {
                        // 有效的基本类型
                    }
                }
            }
        }
    } catch (...) {
        // 捕获主要异常
    }

    // 测试6: 内存压力测试
    try {
        if (Size > 10 && Size < 1000) {  // 避免过大的输入
            std::vector<std::unique_ptr<JsonValue>> values;
            for (int i = 0; i < 10; ++i) {
                try {
                    values.push_back(std::make_unique<JsonValue>(json_string));
                } catch (...) {
                    break;
                }
            }
        }
    } catch (...) {
        // 捕获内存异常
    }

    return 0;
}