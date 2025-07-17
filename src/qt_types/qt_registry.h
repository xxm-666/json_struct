#pragma once
#include "qt_common.h"
#include "qt_ultimate_registry.h"  // Include the ultimate system

namespace JsonStruct {

// Use type traits from qt_common.h
template<typename T>
using type_traits = qt_type_traits<T>;
// Forward declarations
template<typename T> JsonValue qtUniversalToJson(const T& value);
template<typename T> T qtUniversalFromJson(const JsonValue& json, const T& defaultValue);

// Universal Qt value conversion - delegating to ultimate system
template<typename T>
JsonValue qtUniversalToJson(const T& value) {
    return QtUniversal::ultimateToJson(value);
}

template<typename T>
T qtUniversalFromJson(const JsonValue& json, const T& defaultValue) {
    return QtUniversal::ultimateFromJson<T>(json, defaultValue);
}

// Universal Qt container registration - works for any combination
template<typename T>
void registerQtType() {
    TypeRegistry::instance().registerType<T>(
        // toJson
        [](const T& value) -> JsonValue {
            return qtUniversalToJson(value);
        },
        // fromJson
        [](const JsonValue& json, const T& defaultValue) -> T {
            return qtUniversalFromJson<T>(json, defaultValue);
        }
    );
}

// Convenience function for runtime Qt type registration
template<typename T>
void registerCustomQtType() {
    registerQtType<T>();
}

// Legacy function for compatibility
template<typename T>
void registerCustomQtContainer() {
    registerQtType<T>();
}

// Qt type registration
class QtTypesRegistration {
public:
    static void registerAllQtTypes() {
        // Register basic Qt types
        registerQString();
        registerQStringList();
        registerQPointF();
        registerQRectF();
        registerQRect();
        registerQColor();
        registerQSize();
        registerQSizeF();
        
        // Register commonly used Qt container types using universal system
        registerCommonQtContainerTypes();
    }

private:
    // Register common Qt container types using the universal system
    static void registerCommonQtContainerTypes() {
        // Use ultimate universal system for Qt containers
        registerUltimateQtType<QList<int>>();
        registerUltimateQtType<QList<double>>();
        registerUltimateQtType<QList<QString>>();
        registerUltimateQtType<QVector<int>>();
        registerUltimateQtType<QVector<double>>();
        registerUltimateQtType<QVector<QString>>();
        
        // Qt type containers
        registerUltimateQtType<QList<QPointF>>();
        registerUltimateQtType<QList<QRectF>>();
        registerUltimateQtType<QList<QColor>>();
        registerUltimateQtType<QVector<QPointF>>();
        registerUltimateQtType<QVector<QRectF>>();
        registerUltimateQtType<QVector<QColor>>();
        
        // Sets
        registerUltimateQtType<QSet<int>>();
        registerUltimateQtType<QSet<QString>>();
        
        // Maps with string keys
        registerUltimateQtType<QMap<QString, int>>();
        registerUltimateQtType<QMap<QString, QString>>();
        registerUltimateQtType<QMap<QString, QPointF>>();
        registerUltimateQtType<QHash<QString, int>>();
        registerUltimateQtType<QHash<QString, QString>>();
        registerUltimateQtType<QHash<QString, QPointF>>();
        
        // Maps with int keys
        registerUltimateQtType<QMap<int, QString>>();
        registerUltimateQtType<QHash<int, QString>>();
        
        // Pairs
        registerUltimateQtType<QPair<QString, int>>();
        registerUltimateQtType<QPair<QString, QString>>();
        registerUltimateQtType<QPair<QString, QPointF>>();
        registerUltimateQtType<QPair<int, int>>();
    }

private:
    // Register basic Qt types using common implementations
    static void registerQString() {
        TypeRegistry::instance().registerType<QString>(
            [](const QString& str) -> JsonValue {
                return QtBasic::qtBasicToJson(str);
            },
            [](const JsonValue& json, const QString& defaultValue) -> QString {
                return QtBasic::qtBasicFromJson(json, defaultValue);
            }
        );
    }

    static void registerQStringList() {
        TypeRegistry::instance().registerType<QStringList>(
            [](const QStringList& list) -> JsonValue {
                return QtBasic::qtBasicToJson(list);
            },
            [](const JsonValue& json, const QStringList& defaultValue) -> QStringList {
                return QtBasic::qtBasicFromJson(json, defaultValue);
            }
        );
    }

    static void registerQPointF() {
        TypeRegistry::instance().registerType<QPointF>(
            [](const QPointF& point) -> JsonValue {
                return QtBasic::qtBasicToJson(point);
            },
            [](const JsonValue& json, const QPointF& defaultValue) -> QPointF {
                return QtBasic::qtBasicFromJson(json, defaultValue);
            }
        );
    }

    static void registerQRectF() {
        TypeRegistry::instance().registerType<QRectF>(
            [](const QRectF& rect) -> JsonValue {
                return QtBasic::qtBasicToJson(rect);
            },
            [](const JsonValue& json, const QRectF& defaultValue) -> QRectF {
                return QtBasic::qtBasicFromJson(json, defaultValue);
            }
        );
    }

    static void registerQRect() {
        TypeRegistry::instance().registerType<QRect>(
            [](const QRect& rect) -> JsonValue {
                return QtBasic::qtBasicToJson(rect);
            },
            [](const JsonValue& json, const QRect& defaultValue) -> QRect {
                return QtBasic::qtBasicFromJson(json, defaultValue);
            }
        );
    }

    static void registerQColor() {
        TypeRegistry::instance().registerType<QColor>(
            [](const QColor& color) -> JsonValue {
                return QtBasic::qtBasicToJson(color);
            },
            [](const JsonValue& json, const QColor& defaultValue) -> QColor {
                return QtBasic::qtBasicFromJson(json, defaultValue);
            }
        );
    }

    static void registerQSize() {
        TypeRegistry::instance().registerType<QSize>(
            [](const QSize& size) -> JsonValue {
                return QtBasic::qtBasicToJson(size);
            },
            [](const JsonValue& json, const QSize& defaultValue) -> QSize {
                return QtBasic::qtBasicFromJson(json, defaultValue);
            }
        );
    }

    static void registerQSizeF() {
        TypeRegistry::instance().registerType<QSizeF>(
            [](const QSizeF& size) -> JsonValue {
                return QtBasic::qtBasicToJson(size);
            },
            [](const JsonValue& json, const QSizeF& defaultValue) -> QSizeF {
                return QtBasic::qtBasicFromJson(json, defaultValue);
            }
        );
    }
};


// Static initialization for automatic Qt type registration
namespace {
struct QtTypesAutoRegistrar {
    QtTypesAutoRegistrar() {
        QtTypesRegistration::registerAllQtTypes();
    }
};
static QtTypesAutoRegistrar qt_types_auto_registrar;
}

} // namespace JsonStruct
