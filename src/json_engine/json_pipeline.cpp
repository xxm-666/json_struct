#include "json_pipeline.h"
#include <algorithm>
#include <numeric>
#include <sstream>
#include <optional>

namespace JsonStruct {

// Implementation of JsonPipeline::branch member function
JsonPipeline& JsonPipeline::branch(FilterFunction condition, JsonPipeline thenPipe, JsonPipeline elsePipe) {
    steps_.push_back(std::make_unique<BranchStep>(std::move(condition), std::move(thenPipe), std::move(elsePipe)));
    return *this;
}

namespace Transforms {

JsonValue toString(const JsonValue& value) {
    switch (value.type()) {
        case JsonValue::Type::Null:
            return JsonValue("null");
        case JsonValue::Type::Bool:
            return JsonValue(value.toBool() ? "true" : "false");
        case JsonValue::Type::Number:
        {
            std::stringstream ss;
            ss << value.toDouble();
            return JsonValue(ss.str());
        }
        case JsonValue::Type::String:
            return JsonValue(value.toString());
        case JsonValue::Type::Array:
            {
                std::stringstream ss;
                ss << "[";
                if (const auto& arr = value.toArray()) {
                    for (size_t i = 0; i < arr->get().size(); ++i) {
                        if (i > 0) ss << ", ";
                        ss << toString(arr->get()[i]).toString();
                    }
                }
                ss << "]";
                return JsonValue(ss.str());
            }
        case JsonValue::Type::Object:
            return JsonValue("{object}");
        default:
            return JsonValue("unknown");
    }
}

JsonValue toNumber(const JsonValue& value) {
    switch (value.type()) {
        case JsonValue::Type::Number:
            return value;
        case JsonValue::Type::String:
            {
                const std::string& str = value.toString();
                try {
                    if (str.find('.') != std::string::npos || str.find('e') != std::string::npos || str.find('E') != std::string::npos) {
                        double d = std::stod(str);
                        return JsonValue(d);
                    } else {
                        auto i = std::stoll(str);
                        return JsonValue(i);
                    }
                } catch (...) {
                    return JsonValue(JsonNumber::makeNaN());
                }
            }
        case JsonValue::Type::Bool:
            return JsonValue(value.toBool() ? 1 : 0);
        default:
            return JsonValue(JsonNumber::makeNaN());
    }
}

JsonValue toBoolean(const JsonValue& value) {
    switch (value.type()) {
        case JsonValue::Type::Bool:
            return value;
        case JsonValue::Type::Number:
            return JsonValue(value.toDouble() != 0);
        case JsonValue::Type::String:
            return JsonValue(!value.toString().empty());
        case JsonValue::Type::Array:
            if (const auto& arr = value.toArray()) {
                return JsonValue(!arr->get().empty());
            }
            return JsonValue(false);
        case JsonValue::Type::Object:
            if (const auto& obj = value.toObject()) {
                return JsonValue(!obj->get().empty());
            }
            return JsonValue(false);
        case JsonValue::Type::Null:
            return JsonValue(false);
        default:
            return JsonValue(false);
    }
}

JsonValue toImmutable(const JsonValue& value) {
    switch (value.type()) {
        case JsonValue::Type::Array:
            {
                JsonValue::ArrayType arr;
                if(const auto& array = value.toArray()) {
                    for (const auto& item : array->get()) {
                        arr.push_back(toImmutable(item));
                    }
                }
                return JsonValue(std::move(arr));
            }
        case JsonValue::Type::Object:
            {
                JsonValue::ObjectType obj;
                if(const auto& objPtr = value.toObject()) {
                    for (const auto& [key, val] : objPtr->get()) {
                        obj[key] = toImmutable(val);
                    }
                }
                return JsonValue(std::move(obj));
            }
        default:
            return value;
    }
}

JsonStruct::JsonPipeline::TransformFunction query(const std::string& jsonPath) {
    return [jsonPath](const JsonValue& value) -> JsonValue {
        JsonFilter filter;
        auto results = filter.selectValues(value, jsonPath);
        if (results.empty()) return JsonValue();
        if (results.size() == 1) return results[0];
        return JsonValue(results);
    };
}

} // namespace Transforms

namespace Filters {

bool isNotNull(const JsonValue& value) {
    return value.type() != JsonValue::Type::Null;
}

bool isNumber(const JsonValue& value) {
    return value.type() == JsonValue::Type::Number;
}

bool isString(const JsonValue& value) {
    return value.type() == JsonValue::Type::String;
}

bool isArray(const JsonValue& value) {
    return value.type() == JsonValue::Type::Array;
}

bool isObject(const JsonValue& value) {
    return value.type() == JsonValue::Type::Object;
}

JsonStruct::JsonPipeline::FilterFunction arrayLengthGreaterThan(size_t minLength) {
    return [minLength](const JsonValue& value)->bool {
        if (const auto& array = value.toArray()) {
            const auto & arr = array->get();
            return arr.size() > minLength;
        }
        return false;
    };
}

JsonStruct::JsonPipeline::FilterFunction objectHasKey(const std::string& key) {
    return [key](const JsonValue& value) -> bool {
        if (const auto& objOpt = value.toObject()) {
            const auto& obj = objOpt->get();
            return obj.find(key) != obj.end();
        }
        return false;
    };
}

} // namespace Filters

namespace Aggregates {

JsonValue sum(const std::vector<JsonValue>& values) {
    JsonNumber total(0);

    for (const auto& val : values) {
        if (val.type() == JsonValue::Type::Number) {
            JsonNumber num(val.toDouble());
            total = total + num;
        } else if (val.type() == JsonValue::Type::String) {
            try {
                JsonNumber num(std::stod(val.toString()));
                total = total + num;
            } catch (...) {
                // Ignore invalid numbers
            }
        }
    }

    return JsonValue(total);
}

JsonValue average(const std::vector<JsonValue>& values) {
    if (values.empty()) return JsonValue(JsonNumber::makeNaN());

    JsonNumber total(0);
    size_t count = 0;

    for (const auto& val : values) {
        if (val.type() == JsonValue::Type::Number) {
            total = total + JsonNumber(val.toDouble());
            count++;
        }
    }

    if (count == 0) return JsonValue(JsonNumber::makeNaN());
    return JsonValue(JsonNumber(total.toDouble() / count));
}

JsonValue max(const std::vector<JsonValue>& values) {
    if (values.empty()) return JsonValue();

    JsonNumber maxValue(0);
    bool found = false;

    for (const auto& val : values) {
        if (val.type() == JsonValue::Type::Number) {
            JsonNumber num(val.toDouble());
            if (!found || num > maxValue) {
                maxValue = num;
                found = true;
            }
        }
    }

    return found ? JsonValue(maxValue) : JsonValue();
}

JsonValue min(const std::vector<JsonValue>& values) {
    if (values.empty()) return JsonValue();

    JsonNumber minValue = JsonNumber::makeInfinity();

    for (const auto& val : values) {
        if (val.type() == JsonValue::Type::Number) {
            JsonNumber num(val.toDouble());
            if (num < minValue) {
                minValue = num;
            }
        } else if (val.type() == JsonValue::Type::String) {
            try {
                JsonNumber num(std::stod(val.toString()));
                if (num < minValue) {
                    minValue = num;
                }
            } catch (...) {
                // Ignore invalid numbers
            }
        }
    }

    return minValue.isInfinity() ? JsonValue() : JsonValue(minValue);
}

JsonValue count(const std::vector<JsonValue>& values) {
    return JsonValue(static_cast<long long>(values.size()));
}

JsonValue unique(const std::vector<JsonValue>& values) {
    JsonValue::ArrayType result;
    std::set<std::string> seen;

    for (const auto& val : values) {
        std::string str = Transforms::toString(val).toString();
        if (seen.find(str) == seen.end()) {
            seen.insert(str);
            result.push_back(val);
        }
    }

    return JsonValue(std::move(result));
}

} // namespace Aggregates
// Implementation of member query() to delegate to Transforms::query
// Allows JsonPipeline::query(jsonPath) usage
JsonPipeline::TransformFunction JsonPipeline::query(const std::string& jsonPath) {
    return Transforms::query(jsonPath);
}
} // namespace JsonStruct
