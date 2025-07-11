#pragma once

// 包含核心标准C++ JSON框架
#include "json_value.h"
#include "std_type_registry.h"
#include "std_jsonstruct.h"

// 包含标准C++类型注册
#include "std_types_registration.h"

// This file provides a complete standard C++ JSON serialization framework
// No dependency on Qt, only uses standard C++

/*
使用示例:

#include "jsonstruct_std.h"

struct MyConfig {
    std::string name = "default";
    std::vector<std::string> items = {"a", "b", "c"};
    int count = 42;
    double value = 3.14;
    bool enabled = true;
    
    JSON_AUTO(name, items, count, value, enabled)
};

// 使用
MyConfig config;
JsonStruct::JsonObject json = config.toJson();
std::string jsonStr = JsonStruct::JsonValue(json).dump(2);

// 从JSON恢复
JsonStruct::JsonValue parsed = JsonStruct::JsonValue::parse(jsonStr);
config.fromJson(parsed.toObject());
*/
