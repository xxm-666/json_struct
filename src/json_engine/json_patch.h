#pragma once

#include "json_value.h"
#include <string>

namespace JsonStruct
{
namespace JsonPatch
{
    /**
    * @brief Applies a JSON patch to a target JSON value.
    * @param target The target JSON value to apply the patch to.
    * @param patch The JSON patch to apply.
    */
    JsonStruct::JsonValue ApplyPatch(JsonStruct::JsonValue &target, const JsonStruct::JsonValue &patch);
    
    /**
    * @brief Applies a JSONPath to a target JSON value.
    * @param target The target JSON value to apply the path to.
    * @param path The JSONPath to apply.
    */
    JsonStruct::JsonValue ApplyPatch(JsonStruct::JsonValue &target, const std::string &path,
                    const JsonStruct::JsonValue &patch);
}
}
