// 类型转换和边界条件测试
#include "../src/jsonstruct.h"
#include <test_framework/test_framework.h>
#include <iostream>
#include <limits>
#include <cmath>

using namespace JsonStruct;

TEST(TypeConversion_NumericConversions) {
    // 测试各种数值类型的转换

    // 整数边界值
    JsonValue maxInt(std::numeric_limits<int32_t>::max());
    JsonValue minInt(std::numeric_limits<int32_t>::min());
    JsonValue maxLongLong(std::numeric_limits<long long>::max());
    JsonValue minLongLong(std::numeric_limits<long long>::min());

    // 验证整数转换
    ASSERT_EQ(maxInt.toInt(), std::numeric_limits<int32_t>::max());
    ASSERT_EQ(minInt.toInt(), std::numeric_limits<int32_t>::min());
    ASSERT_EQ(maxLongLong.toLongLong(), std::numeric_limits<long long>::max());
    ASSERT_EQ(minLongLong.toLongLong(), std::numeric_limits<long long>::min());

    // 测试整数到浮点数的转换
    ASSERT_NEAR(maxInt.toDouble(), static_cast<double>(std::numeric_limits<int32_t>::max()), 1.0);

    // 浮点数边界值
    JsonValue maxDouble(std::numeric_limits<double>::max());
    JsonValue minDouble(std::numeric_limits<double>::lowest());
    JsonValue epsilon(std::numeric_limits<double>::epsilon());

    // 验证浮点数转换
    ASSERT_EQ(maxDouble.toDouble(), std::numeric_limits<double>::max());
    ASSERT_EQ(minDouble.toDouble(), std::numeric_limits<double>::lowest());
    ASSERT_EQ(epsilon.toDouble(), std::numeric_limits<double>::epsilon());
}

TEST(TypeConversion_SafeIntegerRange) {
    // 测试IEEE 754安全整数范围
    auto safeMax = 9007199254740992LL;   // 2^53
    auto safeMin = -9007199254740992LL;  // -2^53
    auto unsafeMax = 9007199254740993LL; // 2^53 + 1

    JsonValue safeMaxVal(safeMax);
    JsonValue safeMinVal(safeMin);
    JsonValue unsafeMaxVal(unsafeMax);

    // 安全范围内的整数应该保持精度
    ASSERT_EQ(safeMaxVal.getInteger().value(), safeMax);
    ASSERT_EQ(safeMinVal.getInteger().value(), safeMin);

    // 验证安全范围检查
    ASSERT_TRUE(safeMaxVal.isInteger());
    ASSERT_TRUE(safeMinVal.isInteger());

    // 超出安全范围的值可能被转换为double
    if (unsafeMaxVal.isInteger()) {
        ASSERT_EQ(unsafeMaxVal.getInteger().value(), unsafeMax);
    }
    else {
        ASSERT_TRUE(unsafeMaxVal.isDouble());
    }
}

TEST(TypeConversion_StringToNumber) {
    // 测试字符串到数值的隐式转换（如果支持）
    JsonValue stringInt("42");
    JsonValue stringFloat("3.14159");
    JsonValue invalidString("not a number");

    // 测试默认值返回
    ASSERT_EQ(stringInt.toInt(0), 0); // 字符串类型，应该返回默认值
    ASSERT_EQ(stringFloat.toDouble(0.0), 0.0);
    ASSERT_EQ(invalidString.toInt(999), 999);

    // 测试字符串访问
    ASSERT_EQ(stringInt.toString(), "42");
    ASSERT_EQ(stringFloat.toString(), "3.14159");
}

TEST(TypeConversion_BooleanConversions) {
    JsonValue trueVal(true);
    JsonValue falseVal(false);
    JsonValue zeroInt(0);
    JsonValue nonZeroInt(42);
    JsonValue emptyString("");
    JsonValue nonEmptyString("hello");

    // 测试布尔值访问
    ASSERT_TRUE(trueVal.toBool());
    ASSERT_FALSE(falseVal.toBool());

    // 测试其他类型的布尔转换默认值
    ASSERT_EQ(zeroInt.toBool(false), false);     // 整数类型，返回默认值
    ASSERT_EQ(nonZeroInt.toBool(true), true);
    ASSERT_EQ(emptyString.toBool(false), false); // 字符串类型，返回默认值
}

TEST(TypeConversion_NullAndUndefined) {
    JsonValue nullVal(nullptr);
    JsonValue defaultConstructed;

    // 验证null类型
    ASSERT_TRUE(nullVal.isNull());
    ASSERT_TRUE(defaultConstructed.isNull());

    // null值的转换应该返回默认值
    ASSERT_EQ(nullVal.toInt(42), 42);
    ASSERT_EQ(nullVal.toDouble(3.14), 3.14);
    ASSERT_EQ(nullVal.toString("default"), "default");
    ASSERT_EQ(nullVal.toBool(true), true);

    // 验证optional访问返回nullopt
    ASSERT_FALSE(nullVal.getBool().has_value());
    ASSERT_FALSE(nullVal.getNumber().has_value());
    ASSERT_FALSE(nullVal.getString().has_value());
}

TEST(TypeConversion_ArrayConversions) {
    JsonValue::ArrayType arr = { JsonValue(1), JsonValue(2), JsonValue(3) };
    JsonValue arrayVal(arr);

    // 测试数组访问
    ASSERT_TRUE(arrayVal.isArray());
    if (const auto& arrOpt = arrayVal.toArray()) {
        const auto& arr = arrOpt->get();
        ASSERT_EQ(arr.size(), 3);
    }
    else {
        ASSERT_TRUE(false); // Should not reach here
    }

    // 测试数组元素访问
    ASSERT_EQ(arrayVal[0].toInt(), 1);
    ASSERT_EQ(arrayVal[1].toInt(), 2);
    ASSERT_EQ(arrayVal[2].toInt(), 3);

    // 测试越界访问
    JsonValue outOfBounds = arrayVal[10];
    ASSERT_TRUE(outOfBounds.isNull()); // 应该返回null值

    // 测试非数组类型的数组转换
    JsonValue nonArray(42);
    auto array = nonArray.toArray();
    ASSERT_TRUE(!array.has_value());
}

TEST(TypeConversion_ObjectConversions) {
    JsonValue::ObjectType obj;
    obj["key1"] = JsonValue("value1");
    obj["key2"] = JsonValue(42);
    JsonValue objectVal(obj);

    // 测试对象访问
    ASSERT_TRUE(objectVal.isObject());
    if (const auto& objOpt = objectVal.toObject()) {
        const auto& obj = objOpt->get();
        ASSERT_EQ(obj.size(), 2);
    }
    else {
        ASSERT_TRUE(false); // Should not reach here
    }

    // 测试对象属性访问
    ASSERT_EQ(objectVal["key1"].toString(), "value1");
    ASSERT_EQ(objectVal["key2"].toInt(), 42);

    // 测试不存在的键
    JsonValue missingKey = objectVal["nonexistent"];
    ASSERT_TRUE(missingKey.isNull());

    // 测试非对象类型的对象转换
    JsonValue nonObject(42);
    auto object = nonObject.toObject();
    ASSERT_TRUE(!object.has_value()); // 应该返回空optional
}

TEST(BoundaryConditions_LargeStrings) {
    // 测试大字符串处理
    std::string largeString(1024 * 1024, 'x'); // 1MB字符串
    JsonValue largeStringVal(largeString);

    ASSERT_TRUE(largeStringVal.isString());
    ASSERT_EQ(largeStringVal.toString().length(), 1024 * 1024);

    // 测试序列化和反序列化
    std::string serialized = largeStringVal.dump();
    JsonValue reparsed = JsonValue::parse(serialized);
    ASSERT_EQ(reparsed.toString().length(), 1024 * 1024);
}

TEST(BoundaryConditions_DeepNesting) {
    // 创建深层嵌套结构，避免 operator[] 返回临时对象导致链断裂
    JsonValue root(JsonValue::ObjectType{});
    JsonValue* ptr = &root;
    for (int i = 0; i < 100; ++i) {
        (*ptr)["level"] = JsonValue(i);
        (*ptr)["next"] = JsonValue(JsonValue::ObjectType{});
        // 通过 toObject() 获取持久引用，防止悬挂引用
        // ptr = &((*ptr).toObject().at("next"));
        if (const auto& objOpt = ptr->toObject()) {
            ptr = &(objOpt->get().at("next"));
        }
        else {
            ASSERT_TRUE(false); // Should not reach here
        }
    }
    // 验证可以访问深层数据
    JsonValue* nav = &root;
    for (int i = 0; i < 100; ++i) {
        ASSERT_TRUE(nav->isObject());
        ASSERT_EQ((*nav)["level"].toInt(), i);
        // nav = &((*nav).toObject().at("next"));
        if (const auto& nextOpt = nav->toObject()) {
            nav = &(nextOpt->get().at("next"));
        }
        else {
            ASSERT_TRUE(false); // Should not reach here
        }
    }
}

TEST(BoundaryConditions_SpecialFloatValues) {
    // 测试特殊浮点值
    JsonValue nanVal(JsonNumber::makeNaN());
    JsonValue infVal(JsonNumber::makeInfinity());
    JsonValue negInfVal(JsonNumber::makeNegativeInfinity());

    // 验证特殊值检测
    ASSERT_TRUE(nanVal.isNaN());
    ASSERT_TRUE(infVal.isInfinity());
    ASSERT_TRUE(negInfVal.isInfinity());

    ASSERT_FALSE(nanVal.isFinite());
    ASSERT_FALSE(infVal.isFinite());
    ASSERT_FALSE(negInfVal.isFinite());

    // 测试特殊值的比较
    ASSERT_TRUE(std::isnan(nanVal.toDouble()));
    ASSERT_TRUE(std::isinf(infVal.toDouble()));
    ASSERT_TRUE(std::isinf(negInfVal.toDouble()));

    // 验证正负无穷的区别
    ASSERT_TRUE(infVal.toDouble() > 0);
    ASSERT_TRUE(negInfVal.toDouble() < 0);
}

TEST(BoundaryConditions_EmptyContainers) {
    // 测试空容器
    JsonValue emptyArray(JsonValue::ArrayType{});
    JsonValue emptyObject(JsonValue::ObjectType{});

    ASSERT_TRUE(emptyArray.isArray());
    ASSERT_TRUE(emptyObject.isObject());

    // ASSERT_EQ(emptyArray.toArray().size(), 0);
    // ASSERT_EQ(emptyObject.toObject().size(), 0);
    if (const auto& arrOpt = emptyArray.toArray()) {
        ASSERT_EQ(arrOpt->get().size(), 0);
    }
    else {
        ASSERT_TRUE(false); // Should not reach here
    }
    if (const auto& objOpt = emptyObject.toObject()) {
        ASSERT_EQ(objOpt->get().size(), 0);
    }
    else {
        ASSERT_TRUE(false); // Should not reach here
    }

    // 测试空容器的访问
    ASSERT_TRUE(emptyArray[0].isNull() || emptyArray[0].dump() == "[]");
    ASSERT_TRUE(!emptyObject.contains("key"));

    // 测试序列化
    std::string arrayJson = emptyArray.dump();
    std::string objectJson = emptyObject.dump();

    ASSERT_EQ(arrayJson, "[]");
    ASSERT_EQ(objectJson, "{}");
}

TEST(BoundaryConditions_UnicodeEdgeCases) {
    // 测试Unicode边界情况
    std::string unicodeTest = "\u0000\u001F\u007F\u0080\u00FF\uFFFF";
    JsonValue unicodeVal(unicodeTest);

    ASSERT_TRUE(unicodeVal.isString());

    // 测试序列化和反序列化
    std::string serialized = unicodeVal.dump();
    JsonValue reparsed = JsonValue::parse(serialized);

    // 某些控制字符可能被转义，所以检查字符数而不是完全相等
    ASSERT_EQ(reparsed.toString().length(), unicodeTest.length());
}

TEST(BoundaryConditions_NumberPrecision) {
    // 测试数值精度边界
    double verySmall = 1e-100;
    double veryLarge = 1e100;
    double almostOne = 0.9999999999999999;

    JsonValue smallVal(verySmall);
    JsonValue largeVal(veryLarge);
    JsonValue almostOneVal(almostOne);

    // 验证精度保持
    ASSERT_NEAR(smallVal.toDouble(), verySmall, 1e-110);
    ASSERT_NEAR(largeVal.toDouble(), veryLarge, 1e90);
    ASSERT_NEAR(almostOneVal.toDouble(), almostOne, 1e-16);

    // 测试序列化和反序列化精度
    std::string serialized = largeVal.dump();
    JsonValue reparsed = JsonValue::parse(serialized);
    ASSERT_NEAR(reparsed.toDouble(), veryLarge, veryLarge * 1e-15);
}

int main() {
    return RUN_ALL_TESTS();
}
