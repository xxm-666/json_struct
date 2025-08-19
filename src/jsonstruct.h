#pragma once

/**
 * JsonStruct Registry - C++17+ Type Registration and Auto-Serialization System
 * 
 * This is the main entry header file of the project, containing all core features of the type registration system.
 * Users typically only need to include this header to use the full functionality.
 * 
 * Key Features:
 * - Zero-intrusive type registration (JSON_FIELDS macro)
 * - Comprehensive STL type support
 * - Qt type ecosystem integration
 * - Automatic serialization/deserialization
 * - Compile-time type safety
 */

// Core of the type registration system
#include "type_registry/registry_core.h"
#include "type_registry/auto_serializer.h"
#include "type_registry/field_macros.h"

// STL type registration support
#include "std_types/std_registry.h"

// Qt type registration support (if available)
#ifdef QT_CORE_LIB
#include "qt_types/qt_registry.h"
#endif

// JSON engine support (usually not needed directly)
#include "json_engine/json_value.h"

/**
 * Main user interface namespace
 * 
 * User code typically uses this namespace to access all features
 */
namespace JsonStruct {
    // Convenient type aliases
    using Json = JsonValue;
    using JsonObject = JsonValue::ObjectType;
    using JsonArray = JsonValue::ArrayType;
}

class JsonStructRegistrar {
public:
    static void registerAll(bool includeQt = false) {
#ifdef QT_CORE_LIB
        if (includeQt) {
            registerQtTypes();
        }
#endif
        // Standard types are automatically registered via auto_serializer.h and std_registry.h
    }
    
    static void registerStandardTypes() {
    }
    
#ifdef QT_CORE_LIB
    // Register Qt types (only if Qt is available)
    static void registerQtTypes() {
        // Qt Types need explicit registration as they are not standard C++ types
        JsonStruct::QtTypesRegistration::registerAllQtTypes();
    }
#endif
};

/**
 * Quick usage example:
 * 
 * #include "jsonstruct.h"
 * using namespace JsonStruct;
 * 
 * struct UserData {
 *     std::string name;
 *     std::vector<int> scores;
 *     std::map<std::string, double> settings;
 *     
 *     JSON_AUTO(UserData, name, scores, settings)
 * };
 * 
 * int main() {
 *     UserData data;
 *     std::string json = data.toJsonString(2);
 *     UserData restored = UserData::fromJsonString(json);
 *     return 0;
 * }
 */
