#pragma once
#include "../json_engine/json_value.h"
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <any>
#include <memory>

namespace JsonStruct {

// Type serializer interface
class ITypeSerializer {
public:
    virtual ~ITypeSerializer() = default;
    virtual JsonValue toJson(const std::any& value) const = 0;
    virtual std::any fromJson(const JsonValue& json, const std::any& defaultValue) const = 0;
    virtual std::type_index getTypeIndex() const = 0;
};

// Templated type serializer implementation
template<typename T>
class TypeSerializer : public ITypeSerializer {
public:
    using ToJsonFunc = std::function<JsonValue(const T&)>;
    using FromJsonFunc = std::function<T(const JsonValue&, const T&)>;
    
    TypeSerializer(ToJsonFunc toJson, FromJsonFunc fromJson)
        : toJsonFunc_(std::move(toJson)), fromJsonFunc_(std::move(fromJson)) {}
    
    JsonValue toJson(const std::any& value) const override {
        try {
            const T& typedValue = std::any_cast<const T&>(value);
            return toJsonFunc_(typedValue);
        } catch (const std::bad_any_cast&) {
            return JsonValue();
        }
    }
    
    std::any fromJson(const JsonValue& json, const std::any& defaultValue) const override {
        try {
            const T& typedDefault = std::any_cast<const T&>(defaultValue);
            return fromJsonFunc_(json, typedDefault);
        } catch (const std::bad_any_cast&) {
            return defaultValue;
        }
    }
    
    std::type_index getTypeIndex() const override {
        return std::type_index(typeid(T));
    }
    
private:
    ToJsonFunc toJsonFunc_;
    FromJsonFunc fromJsonFunc_;
};

// Type registry
class TypeRegistry {
public:
    static TypeRegistry& instance() {
        static TypeRegistry registry;
        return registry;
    }
    
    // Register type serializer
    template<typename T>
    void registerType(
        std::function<JsonValue(const T&)> toJson,
        std::function<T(const JsonValue&, const T&)> fromJson
    ) {
        auto serializer = std::make_unique<TypeSerializer<T>>(
            std::move(toJson), std::move(fromJson)
        );
        
        std::type_index typeIdx(typeid(T));
        serializers_[typeIdx] = std::move(serializer);
    }
    
    // Check if type is registered
    template<typename T>
    bool isRegistered() const {
        std::type_index typeIdx(typeid(T));
        return serializers_.find(typeIdx) != serializers_.end();
    }
    
    // Serialize registered types
    template<typename T>
    JsonValue toJson(const T& value) const {
        std::type_index typeIdx(typeid(T));
        auto it = serializers_.find(typeIdx);
        if (it != serializers_.end()) {
            return it->second->toJson(std::make_any<T>(value));
        }
        return JsonValue(); // Type not registered
    }
    
    // Deserialize registered types
    template<typename T>
    T fromJson(const JsonValue& json, const T& defaultValue = T{}) const {
        std::type_index typeIdx(typeid(T));
        auto it = serializers_.find(typeIdx);
        if (it != serializers_.end()) {
            std::any result = it->second->fromJson(json, std::make_any<T>(defaultValue));
            try {
                return std::any_cast<T>(result);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue; // Type not registered
    }
    
    // Get all registered types
    std::vector<std::type_index> getRegisteredTypes() const {
        std::vector<std::type_index> types;
        for (const auto& pair : serializers_) {
            types.push_back(pair.first);
        }
        return types;
    }
    
    // Clear all registrations
    void clear() {
        serializers_.clear();
    }
    
private:
    TypeRegistry() = default;
    std::unordered_map<std::type_index, std::unique_ptr<ITypeSerializer>> serializers_;
};

/// Simple type registration macro
#define REGISTER_JSON_TYPE(Type, toJsonImpl, fromJsonImpl) \
    namespace { \
        struct Type##Registrar { \
            Type##Registrar() { \
                JsonStruct::TypeRegistry::instance().registerType<Type>( \
                    toJsonImpl, \
                    fromJsonImpl \
                ); \
            } \
        }; \
        static Type##Registrar Type##_registrar_instance; \
    }

// Convenience macro: Simple type registration (for basic types)
#define REGISTER_SIMPLE_JSON_TYPE(Type, jsonConversion) \
    REGISTER_JSON_TYPE(Type, \
        [](const Type& val) -> JsonValue { return JsonValue(jsonConversion(val)); }, \
        [](const JsonValue& json, const Type& defaultVal) -> Type { \
            return json.isNull() ? defaultVal : static_cast<Type>(json.jsonConversion(defaultVal)); \
        } \
    )

} // namespace JsonStruct
