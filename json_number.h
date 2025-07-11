#pragma once
#include <cstdint>
#include <string>
#include <optional>
#include <stdexcept>
#include <climits>
#include <cmath>
#include <cfloat>

namespace JsonStruct {

/**
 * @brief 高精度JSON数值类型，解决long long->double精度丢失问题
 * 
 * 这个类专门设计来处理JSON中的数值类型，支持：
 * - 64位整数(int64_t)的精确存储和操作
 * - 64位浮点数(double)的精确存储和操作  
 * - 安全的类型转换，避免精度丢失
 * - IEEE 754 安全整数范围检查
 */
class JsonNumber {
public:
    enum Type {
        Integer,    ///< 64位整数类型
        Double      ///< 64位浮点数类型
    };
    
    // IEEE 754 double 的安全整数范围
    static constexpr int64_t SAFE_INTEGER_MAX = 9007199254740992LL;   // 2^53
    static constexpr int64_t SAFE_INTEGER_MIN = -9007199254740992LL;  // -2^53

private:
    Type type_;
    union {
        int64_t intValue_;
        double doubleValue_;
    };

public:
    // 构造函数
    JsonNumber() : type_(Integer), intValue_(0) {}
    JsonNumber(int32_t val) : type_(Integer), intValue_(val) {}
    JsonNumber(int64_t val) : type_(Integer), intValue_(val) {}
    JsonNumber(uint32_t val) : type_(Integer), intValue_(static_cast<int64_t>(val)) {}
    JsonNumber(uint64_t val) {
        if (val <= static_cast<uint64_t>(INT64_MAX)) {
            type_ = Integer;
            intValue_ = static_cast<int64_t>(val);
        } else {
            type_ = Double;
            doubleValue_ = static_cast<double>(val);
        }
    }
    JsonNumber(float val) : type_(Double), doubleValue_(static_cast<double>(val)) {}
    JsonNumber(double val) : type_(Double), doubleValue_(val) {}

    // 拷贝构造和赋值
    JsonNumber(const JsonNumber& other) : type_(other.type_) {
        if (type_ == Integer) {
            intValue_ = other.intValue_;
        } else {
            doubleValue_ = other.doubleValue_;
        }
    }

    JsonNumber& operator=(const JsonNumber& other) {
        if (this != &other) {
            type_ = other.type_;
            if (type_ == Integer) {
                intValue_ = other.intValue_;
            } else {
                doubleValue_ = other.doubleValue_;
            }
        }
        return *this;
    }

    // 移动构造和赋值
    JsonNumber(JsonNumber&& other) noexcept : type_(other.type_) {
        if (type_ == Integer) {
            intValue_ = other.intValue_;
        } else {
            doubleValue_ = other.doubleValue_;
        }
    }

    JsonNumber& operator=(JsonNumber&& other) noexcept {
        if (this != &other) {
            type_ = other.type_;
            if (type_ == Integer) {
                intValue_ = other.intValue_;
            } else {
                doubleValue_ = other.doubleValue_;
            }
        }
        return *this;
    }

    // 析构函数
    ~JsonNumber() = default;

    // 类型查询
    Type getType() const noexcept { return type_; }
    bool isInteger() const noexcept { return type_ == Integer; }
    bool isDouble() const noexcept { return type_ == Double; }

    // 安全获取原始值
    std::optional<int64_t> getInteger() const noexcept {
        if (type_ == Integer) {
            return intValue_;
        }
        if (type_ == Double && doubleValue_ == std::floor(doubleValue_)) {
            // 检查double是否在安全整数范围内
            if (doubleValue_ >= static_cast<double>(LLONG_MIN) && 
                doubleValue_ <= static_cast<double>(LLONG_MAX)) {
                return static_cast<int64_t>(doubleValue_);
            }
        }
        return std::nullopt;
    }

    std::optional<double> getDouble() const noexcept {
        if (type_ == Double) {
            return doubleValue_;
        }
        if (type_ == Integer) {
            return static_cast<double>(intValue_);
        }
        return std::nullopt;
    }

    // 强制类型转换（带范围检查）
    int64_t toInteger() const {
        if (type_ == Integer) {
            return intValue_;
        }
        if (type_ == Double) {
            if (doubleValue_ >= static_cast<double>(LLONG_MIN) && 
                doubleValue_ <= static_cast<double>(LLONG_MAX) &&
                doubleValue_ == std::floor(doubleValue_)) {
                return static_cast<int64_t>(doubleValue_);
            }
            throw std::overflow_error("Double value cannot be safely converted to integer");
        }
        return 0;
    }

    double toDouble() const {
        if (type_ == Double) {
            return doubleValue_;
        }
        if (type_ == Integer) {
            // 检查整数是否在安全double范围内
            if (intValue_ > SAFE_INTEGER_MAX || intValue_ < SAFE_INTEGER_MIN) {
                // 注意：这里会有精度丢失，但这是从整数到浮点数的常见情况
            }
            return static_cast<double>(intValue_);
        }
        return 0.0;
    }

    // 类型转换（带默认值）
    int32_t toInt32(int32_t defaultValue = 0) const noexcept {
        if (type_ == Integer) {
            if (intValue_ >= INT32_MIN && intValue_ <= INT32_MAX) {
                return static_cast<int32_t>(intValue_);
            }
        } else if (type_ == Double) {
            if (doubleValue_ >= INT32_MIN && doubleValue_ <= INT32_MAX &&
                doubleValue_ == std::floor(doubleValue_)) {
                return static_cast<int32_t>(doubleValue_);
            }
        }
        return defaultValue;
    }

    int64_t toInt64(int64_t defaultValue = 0) const noexcept {
        try {
            return toInteger();
        } catch (...) {
            return defaultValue;
        }
    }

    float toFloat(float defaultValue = 0.0f) const noexcept {
        if (type_ == Double) {
            if (doubleValue_ >= -FLT_MAX && doubleValue_ <= FLT_MAX) {
                return static_cast<float>(doubleValue_);
            }
        } else if (type_ == Integer) {
            return static_cast<float>(intValue_);
        }
        return defaultValue;
    }

    // 精度检查函数
    bool isInSafeIntegerRange() const noexcept {
        if (type_ == Integer) {
            return intValue_ >= SAFE_INTEGER_MIN && intValue_ <= SAFE_INTEGER_MAX;
        }
        return false;
    }

    bool canConvertToIntegerSafely() const noexcept {
        if (type_ == Integer) {
            return true;
        }
        if (type_ == Double) {
            return doubleValue_ == std::floor(doubleValue_) &&
                   doubleValue_ >= static_cast<double>(LLONG_MIN) &&
                   doubleValue_ <= static_cast<double>(LLONG_MAX);
        }
        return false;
    }

    bool canConvertToDoubleSafely() const noexcept {
        if (type_ == Double) {
            return true;
        }
        if (type_ == Integer) {
            return intValue_ >= SAFE_INTEGER_MIN && intValue_ <= SAFE_INTEGER_MAX;
        }
        return false;
    }

    // 序列化
    std::string toString() const {
        if (type_ == Integer) {
            return std::to_string(intValue_);
        } else {
            // 使用高精度格式化避免科学计数法
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%.15g", doubleValue_);
            return std::string(buffer);
        }
    }

    // 比较操作符
    bool operator==(const JsonNumber& other) const noexcept {
        if (type_ == other.type_) {
            if (type_ == Integer) {
                return intValue_ == other.intValue_;
            } else {
                return doubleValue_ == other.doubleValue_;
            }
        }
        
        // 跨类型比较
        if (type_ == Integer && other.type_ == Double) {
            return static_cast<double>(intValue_) == other.doubleValue_;
        }
        if (type_ == Double && other.type_ == Integer) {
            return doubleValue_ == static_cast<double>(other.intValue_);
        }
        
        return false;
    }

    bool operator!=(const JsonNumber& other) const noexcept {
        return !(*this == other);
    }

    bool operator<(const JsonNumber& other) const noexcept {
        if (type_ == Integer && other.type_ == Integer) {
            return intValue_ < other.intValue_;
        }
        if (type_ == Double && other.type_ == Double) {
            return doubleValue_ < other.doubleValue_;
        }
        
        // 跨类型比较
        double thisVal = toDouble();
        double otherVal = other.toDouble();
        return thisVal < otherVal;
    }

    bool operator<=(const JsonNumber& other) const noexcept {
        return *this < other || *this == other;
    }

    bool operator>(const JsonNumber& other) const noexcept {
        return !(*this <= other);
    }

    bool operator>=(const JsonNumber& other) const noexcept {
        return !(*this < other);
    }

    // 数学运算
    JsonNumber operator+(const JsonNumber& other) const {
        if (type_ == Integer && other.type_ == Integer) {
            // 检查整数溢出
            if ((other.intValue_ > 0 && intValue_ > INT64_MAX - other.intValue_) ||
                (other.intValue_ < 0 && intValue_ < INT64_MIN - other.intValue_)) {
                // 溢出，使用double
                return JsonNumber(static_cast<double>(intValue_) + static_cast<double>(other.intValue_));
            }
            return JsonNumber(intValue_ + other.intValue_);
        }
        
        // 有浮点数参与，使用double运算
        return JsonNumber(toDouble() + other.toDouble());
    }

    JsonNumber operator-(const JsonNumber& other) const {
        if (type_ == Integer && other.type_ == Integer) {
            // 检查整数溢出
            if ((other.intValue_ < 0 && intValue_ > INT64_MAX + other.intValue_) ||
                (other.intValue_ > 0 && intValue_ < INT64_MIN + other.intValue_)) {
                // 溢出，使用double
                return JsonNumber(static_cast<double>(intValue_) - static_cast<double>(other.intValue_));
            }
            return JsonNumber(intValue_ - other.intValue_);
        }
        
        // 有浮点数参与，使用double运算
        return JsonNumber(toDouble() - other.toDouble());
    }

    JsonNumber operator*(const JsonNumber& other) const {
        if (type_ == Integer && other.type_ == Integer) {
            // 检查整数溢出
            if (intValue_ != 0 && other.intValue_ != 0) {
                if ((intValue_ > 0 && other.intValue_ > 0 && intValue_ > INT64_MAX / other.intValue_) ||
                    (intValue_ < 0 && other.intValue_ < 0 && intValue_ < INT64_MAX / other.intValue_) ||
                    (intValue_ > 0 && other.intValue_ < 0 && other.intValue_ < INT64_MIN / intValue_) ||
                    (intValue_ < 0 && other.intValue_ > 0 && intValue_ < INT64_MIN / other.intValue_)) {
                    // 溢出，使用double
                    return JsonNumber(static_cast<double>(intValue_) * static_cast<double>(other.intValue_));
                }
            }
            return JsonNumber(intValue_ * other.intValue_);
        }
        
        // 有浮点数参与，使用double运算
        return JsonNumber(toDouble() * other.toDouble());
    }

    JsonNumber operator/(const JsonNumber& other) const {
        if (other == JsonNumber(0)) {
            throw std::runtime_error("Division by zero");
        }
        
        // 除法总是返回double以避免精度问题
        return JsonNumber(toDouble() / other.toDouble());
    }

    // 调试信息
    std::string debugInfo() const {
        std::string info = "JsonNumber{type=";
        info += (type_ == Integer) ? "Integer" : "Double";
        info += ", value=";
        info += toString();
        if (type_ == Integer) {
            info += ", inSafeRange=" + std::string(isInSafeIntegerRange() ? "true" : "false");
        }
        info += "}";
        return info;
    }
};

} // namespace JsonStruct
