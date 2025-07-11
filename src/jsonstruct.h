#pragma once

/**
 * JsonStruct Registry - C++17+ 类型注册与自动序列化系统
 * 
 * 这是项目的主入口头文件，包含了类型注册系统的所有核心功能。
 * 用户通常只需要包含这一个头文件即可使用完整功能。
 * 
 * 核心特性：
 * - 零侵入类型注册 (JSON_FIELDS宏)
 * - 全面STL类型支持
 * - Qt类型生态集成
 * - 自动序列化/反序列化
 * - 编译时类型安全
 */

// 🎯 类型注册系统核心
#include "type_registry/registry_core.h"
#include "type_registry/auto_serializer.h"
#include "type_registry/field_macros.h"

// 📦 STL类型注册支持
#include "std_types/std_registry.h"

// 🖼️ Qt类型注册支持 (如果可用)
#ifdef QT_CORE_LIB
#include "qt_types/qt_registry.h"
#endif

// 🔧 JSON引擎支撑 (通常不需要直接使用)
#include "json_engine/json_value.h"

/**
 * 🎨 主要用户接口命名空间
 * 
 * 用户代码通常使用这个命名空间来访问所有功能
 */
namespace JsonStruct {
    // 提供便捷的类型别名
    using Json = JsonValue;
    using JsonObject = JsonValue::ObjectType;
    using JsonArray = JsonValue::ArrayType;
}

/**
 * 🚀 快速使用示例:
 * 
 * #include "jsonstruct.h"
 * using namespace JsonStruct;
 * 
 * struct UserData {
 *     std::string name;
 *     std::vector<int> scores;
 *     std::map<std::string, double> settings;
 *     
 *     JSON_FIELDS(name, scores, settings)
 * };
 * 
 * int main() {
 *     UserData data;
 *     std::string json = data.toJsonString(2);
 *     UserData restored = UserData::fromJsonString(json);
 *     return 0;
 * }
 */
