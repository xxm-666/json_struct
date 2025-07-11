#pragma once

/**
 * JSON_FIELDS 宏定义 - 类型注册的核心宏
 * 
 * 这个头文件包含了用户最常用的 JSON_FIELDS 宏，
 * 它是整个类型注册系统的用户接口核心。
 */

#include "../json_engine/json_value.h"
#include <tuple>
#include <sstream>
#include <vector>
#include <string>

namespace JsonStruct {
namespace FieldMacros {

/**
 * 辅助函数：解析字段名字符串
 */
inline std::vector<std::string> split_field_names(const std::string& names) {
    std::vector<std::string> result;
    std::stringstream ss(names);
    std::string item;
    while (std::getline(ss, item, ',')) {
        size_t start = item.find_first_not_of(" \t");
        if (start != std::string::npos) {
            size_t end = item.find_last_not_of(" \t");
            result.push_back(item.substr(start, end - start + 1));
        }
    }
    return result;
}

} // namespace FieldMacros
} // namespace JsonStruct

/**
 * 🎯 JSON_FIELDS 宏 - 一行代码完成类型注册
 * 
 * 这是用户最重要的接口，通过这个宏可以为任何结构体
 * 快速添加JSON序列化/反序列化能力。
 * 
 * 使用方法：
 * struct MyStruct {
 *     std::string name;
 *     int age;
 *     std::vector<double> scores;
 *     
 *     JSON_FIELDS(name, age, scores)
 * };
 */
#define JSON_FIELDS(...)                                  \
    auto json_fields() const {                            \
        return std::tie(__VA_ARGS__);                     \
    }                                                     \
    auto json_fields() {                                  \
        return std::tie(__VA_ARGS__);                     \
    }                                                     \
    static const char* json_field_names() {              \
        return #__VA_ARGS__;                              \
    }                                                     \
    static std::vector<std::string> get_field_names() {  \
        return JsonStruct::FieldMacros::split_field_names(json_field_names()); \
    }

/**
 * 🔧 高级宏：自定义序列化行为
 * 
 * 为需要特殊处理的类型提供更多控制
 */
#define JSON_FIELDS_WITH_OPTIONS(options, ...)           \
    JSON_FIELDS(__VA_ARGS__)                             \
    static constexpr auto json_options() { return options; }

/**
 * 📦 便捷宏：只读序列化
 * 
 * 仅支持序列化，不支持反序列化
 */
#define JSON_FIELDS_READONLY(...)                        \
    auto json_fields() const {                           \
        return std::tie(__VA_ARGS__);                    \
    }                                                    \
    static const char* json_field_names() {             \
        return #__VA_ARGS__;                             \
    }                                                    \
    std::string toJsonString(int indent = 0) const

/**
 * 🎨 使用示例和最佳实践：
 * 
 * // 基础用法
 * struct User {
 *     std::string name;
 *     int age;
 *     JSON_FIELDS(name, age)
 * };
 * 
 * // 复杂嵌套类型
 * struct Config {
 *     std::map<std::string, std::vector<int>> data;
 *     std::optional<User> admin;
 *     std::shared_ptr<std::vector<std::string>> tags;
 *     JSON_FIELDS(data, admin, tags)
 * };
 * 
 * // 只读序列化
 * struct ReadOnlyData {
 *     std::string computed_value;
 *     JSON_FIELDS_READONLY(computed_value)
 * };
 */
