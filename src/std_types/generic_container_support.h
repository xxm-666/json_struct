#pragma once
#include "../json_engine/json_value.h"
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <utility>
#include <type_traits>

namespace JsonStruct {

// 检测容器类型的工具模板
template<typename T>
struct is_vector : std::false_type {};

template<typename T, typename A>
struct is_vector<std::vector<T, A>> : std::true_type {
    using value_type = T;
};

template<typename T>
struct is_list : std::false_type {};

template<typename T, typename A>
struct is_list<std::list<T, A>> : std::true_type {
    using value_type = T;
};

template<typename T>
struct is_deque : std::false_type {};

template<typename T, typename A>
struct is_deque<std::deque<T, A>> : std::true_type {
    using value_type = T;
};

template<typename T>
struct is_set : std::false_type {};

template<typename T, typename C, typename A>
struct is_set<std::set<T, C, A>> : std::true_type {
    using value_type = T;
};

template<typename T>
struct is_unordered_set : std::false_type {};

template<typename T, typename H, typename E, typename A>
struct is_unordered_set<std::unordered_set<T, H, E, A>> : std::true_type {
    using value_type = T;
};

template<typename T>
struct is_map : std::false_type {};

template<typename K, typename V, typename C, typename A>
struct is_map<std::map<K, V, C, A>> : std::true_type {
    using key_type = K;
    using mapped_type = V;
};

template<typename T>
struct is_unordered_map : std::false_type {};

template<typename K, typename V, typename H, typename E, typename A>
struct is_unordered_map<std::unordered_map<K, V, H, E, A>> : std::true_type {
    using key_type = K;
    using mapped_type = V;
};

template<typename T>
struct is_pair : std::false_type {};

template<typename T1, typename T2>
struct is_pair<std::pair<T1, T2>> : std::true_type {
    using first_type = T1;
    using second_type = T2;
};

// 组合类型检测
template<typename T>
constexpr bool is_sequence_container_v = 
    is_vector<T>::value || 
    is_list<T>::value || 
    is_deque<T>::value;

template<typename T>
constexpr bool is_associative_container_v = 
    is_set<T>::value || 
    is_unordered_set<T>::value;

template<typename T>
constexpr bool is_map_container_v = 
    is_map<T>::value || 
    is_unordered_map<T>::value;

template<typename T>
constexpr bool is_container_v = 
    is_sequence_container_v<T> || 
    is_associative_container_v<T> || 
    is_map_container_v<T>;

// 前向声明
template<typename T> JsonValue genericToJsonValue(const T& value);
template<typename T> T genericFromJsonValue(const JsonValue& json, const T& defaultValue);

// 序列容器的通用序列化 (vector, list, deque)
template<typename Container>
typename std::enable_if<is_sequence_container_v<Container>, JsonValue>::type
containerToJson(const Container& container) {
    JsonValue::ArrayType arr;
    arr.reserve(container.size());
    for (const auto& item : container) {
        arr.push_back(genericToJsonValue(item));
    }
    return JsonValue(arr);
}

// 序列容器的通用反序列化
template<typename Container>
typename std::enable_if<is_sequence_container_v<Container>, Container>::type
containerFromJson(const JsonValue& json, const Container& defaultValue) {
    if (!json.isArray()) {
        return defaultValue;
    }
    
    Container result;
    const auto& arr = json.toArray();
    
    if constexpr (is_vector<Container>::value || is_deque<Container>::value) {
        result.reserve(arr.size());
    }
    
    for (const auto& item : arr) {
        using ValueType = typename Container::value_type;
        result.insert(result.end(), genericFromJsonValue(item, ValueType{}));
    }
    
    return result;
}

// 关联容器的通用序列化 (set, unordered_set)
template<typename Container>
typename std::enable_if<is_associative_container_v<Container>, JsonValue>::type
containerToJson(const Container& container) {
    JsonValue::ArrayType arr;
    for (const auto& item : container) {
        arr.push_back(genericToJsonValue(item));
    }
    return JsonValue(arr);
}

// 关联容器的通用反序列化
template<typename Container>
typename std::enable_if<is_associative_container_v<Container>, Container>::type
containerFromJson(const JsonValue& json, const Container& defaultValue) {
    if (!json.isArray()) {
        return defaultValue;
    }
    
    Container result;
    const auto& arr = json.toArray();
    
    for (const auto& item : arr) {
        using ValueType = typename Container::value_type;
        result.insert(genericFromJsonValue(item, ValueType{}));
    }
    
    return result;
}

// Map容器的通用序列化 (map, unordered_map)
template<typename Container>
typename std::enable_if<is_map_container_v<Container>, JsonValue>::type
containerToJson(const Container& container) {
    // 如果key是string类型，使用JSON对象
    if constexpr (std::is_same_v<typename Container::key_type, std::string>) {
        JsonValue::ObjectType obj;
        for (const auto& [key, value] : container) {
            obj[key] = genericToJsonValue(value);
        }
        return JsonValue(obj);
    }
    // 否则使用数组的数组格式 [[key, value], ...]
    else {
        JsonValue::ArrayType arr;
        for (const auto& [key, value] : container) {
            JsonValue::ArrayType pair;
            pair.push_back(genericToJsonValue(key));
            pair.push_back(genericToJsonValue(value));
            arr.push_back(JsonValue(pair));
        }
        return JsonValue(arr);
    }
}

// Map容器的通用反序列化
template<typename Container>
typename std::enable_if<is_map_container_v<Container>, Container>::type
containerFromJson(const JsonValue& json, const Container& defaultValue) {
    Container result;
    
    // 如果key是string类型，从JSON对象解析
    if constexpr (std::is_same_v<typename Container::key_type, std::string>) {
        if (json.isObject()) {
            const auto& obj = json.toObject();
            for (const auto& [key, value] : obj) {
                using ValueType = typename Container::mapped_type;
                result[key] = genericFromJsonValue(value, ValueType{});
            }
        }
    }
    // 否则从数组的数组格式解析
    else {
        if (json.isArray()) {
            const auto& arr = json.toArray();
            for (const auto& pairJson : arr) {
                if (pairJson.isArray()) {
                    const auto& pairArr = pairJson.toArray();
                    if (pairArr.size() >= 2) {
                        using KeyType = typename Container::key_type;
                        using ValueType = typename Container::mapped_type;
                        auto key = genericFromJsonValue(pairArr[0], KeyType{});
                        auto value = genericFromJsonValue(pairArr[1], ValueType{});
                        result[key] = value;
                    }
                }
            }
        }
    }
    
    return result.empty() ? defaultValue : result;
}

// pair的序列化
template<typename T1, typename T2>
JsonValue pairToJson(const std::pair<T1, T2>& p) {
    JsonValue::ArrayType arr;
    arr.push_back(genericToJsonValue(p.first));
    arr.push_back(genericToJsonValue(p.second));
    return JsonValue(arr);
}

// pair的反序列化
template<typename T1, typename T2>
std::pair<T1, T2> pairFromJson(const JsonValue& json, const std::pair<T1, T2>& defaultValue) {
    if (json.isArray()) {
        const auto& arr = json.toArray();
        if (arr.size() >= 2) {
            return std::make_pair(
                genericFromJsonValue(arr[0], T1{}),
                genericFromJsonValue(arr[1], T2{})
            );
        }
    }
    return defaultValue;
}

} // namespace JsonStruct
