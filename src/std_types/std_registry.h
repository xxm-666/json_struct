#pragma once

// JsonStruct 统一类型注册系统 - 简化版
// 基于模板的自动类型处理，无需显式注册

#include "../json_engine/json_value.h"
#include "../type_registry/registry_core.h"
#include "../type_registry/auto_serializer.h"
#include "generic_container_support.h"

// 条件包含Qt支持
#ifdef QT_CORE_LIB
#include "../qt_types/qt_registry.h"
#endif

namespace JsonStruct {

// 简化的类型系统接口
class JsonStructRegistrar {
public:
    // 注册函数（主要用于Qt类型和向后兼容）
    static void registerAll(bool includeQt = false) {
#ifdef QT_CORE_LIB
        if (includeQt) {
            registerQtTypes();
        }
#endif
        // 标准C++类型通过模板自动处理，无需显式注册
    }
    
    // 只注册标准C++类型（实际上什么都不做，因为是自动的）
    static void registerStandardTypes() {
        // 标准C++类型通过 auto_serializer.h 和 generic_container_support.h 自动处理
        // 此方法保留用于向后兼容
    }
    
#ifdef QT_CORE_LIB
    // 注册Qt类型（仅在Qt可用时）
    static void registerQtTypes() {
        // Qt类型需要显式注册，因为它们不是标准C++类型
        QtTypesRegistration::registerAllQtTypes();
    }
#endif
};

} // namespace JsonStruct
