#pragma once

/**
 * JSON_FIELDS macro definition - Core macro for type registration
 * 
 * This header contains the most commonly used JSON_FIELDS macro,
 * which is the core user interface for the type registration system.
 */

#include "../json_engine/json_value.h"
#include <tuple>
#include <sstream>
#include <vector>
#include <string>

namespace JsonStruct {
namespace FieldMacros {

/**
 * Helper function: Parse field name string
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
 * 🔧 Advanced macro for custom serialization behavior.
 * 
 * Provides more control for types that need special handling.
 */
#define JSON_FIELDS_WITH_OPTIONS(options, ...)           \
    JSON_FIELDS(__VA_ARGS__)                             \
    static constexpr auto json_options() { return options; }

/**
 * 🎨 Usage examples and best practices:
 * 
 * // Basic usage
 * struct User {
 *     std::string name;
 *     int age;
 *     JSON_FIELDS(name, age)
 * };
 * 
 * // Complex nested type
 * struct Config {
 *     std::map<std::string, std::vector<int>> data;
 *     std::optional<User> admin;
 *     std::shared_ptr<std::vector<std::string>> tags;
 *     JSON_FIELDS(data, admin, tags)
 * };
 * 
 */
