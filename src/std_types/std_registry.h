#pragma once

// Include core standard C++ JSON framework
#include "../json_engine/json_value.h"
#include "../type_registry/registry_core.h"
#include "../type_registry/auto_serializer.h"

// Include standard C++ type registration
#include "container_registry.h"

// This file provides a complete standard C++ JSON serialization framework
// No dependency on Qt, only uses standard C++

/*
Usage example:

#include "jsonstruct_std.h"

struct MyConfig {
    std::string name = "default";
    std::vector<std::string> items = {"a", "b", "c"};
    int count = 42;
    double value = 3.14;
    bool enabled = true;
    
    JSON_AUTO(name, items, count, value, enabled)
};

// Usage
MyConfig config;
JsonStruct::JsonObject json = config.toJson();
std::string jsonStr = JsonStruct::JsonValue(json).dump(2);

// Restore from JSON
JsonStruct::JsonValue parsed = JsonStruct::JsonValue::parse(jsonStr);
config.fromJson(parsed.toObject());
*/
