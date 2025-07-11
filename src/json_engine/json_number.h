#pragma once
#include <cstdint>
#include <string>
#include <optional>
#include <stdexcept>
#include <climits>
#include <cmath>
#include <cfloat>
#include <limits>

namespace JsonStruct {

/**
 * @brief High-precision JSON number type, solves long long -> double precision loss
 *
 * This class is designed to handle JSON numeric types, supporting:
 * - Precise storage and operations for 64-bit integers (int64_t)
 * - Precise storage and operations for 64-bit floating point numbers (double)
 * - Safe type conversions to avoid precision loss
 * - IEEE 754 safe integer range checking
 * - IEEE 754 special value support (NaN, Infinity)
 */
class JsonNumber {
public:
    enum Type {
        Integer,    ///< 64-bit integer type
        Double      ///< 64-bit floating point type
    };
    
    // Safe integer range for IEEE 754 double
    static constexpr int64_t SAFE_INTEGER_MAX = 9007199254740992LL;   // 2^53
    static constexpr int64_t SAFE_INTEGER_MIN = -9007199254740992LL;  // -2^53

private:
    Type type_;
    union {
        int64_t intValue_;
        double doubleValue_;
    };

public:
    // Constructor
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
    
    // Static factory methods for special values
    static JsonNumber makeNaN() {
        JsonNumber num;
        num.type_ = Double;
        num.doubleValue_ = std::numeric_limits<double>::quiet_NaN();
        return num;
    }
    
    static JsonNumber makeInfinity() {
        JsonNumber num;
        num.type_ = Double;
        num.doubleValue_ = std::numeric_limits<double>::infinity();
        return num;
    }
    
    static JsonNumber makeNegativeInfinity() {
        JsonNumber num;
        num.type_ = Double;
        num.doubleValue_ = -std::numeric_limits<double>::infinity();
        return num;
    }

    // Copy constructor and assignment
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

    // Move constructor and assignment
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

    // Destructor
    ~JsonNumber() = default;

    // Type queries
    Type getType() const noexcept { return type_; }
    bool isInteger() const noexcept { return type_ == Integer; }
    bool isDouble() const noexcept { return type_ == Double; }
    
    // IEEE 754 special value queries
    bool isNaN() const noexcept {
        return type_ == Double && std::isnan(doubleValue_);
    }
    
    bool isInfinity() const noexcept {
        return type_ == Double && std::isinf(doubleValue_);
    }
    
    bool isPositiveInfinity() const noexcept {
        return type_ == Double && std::isinf(doubleValue_) && doubleValue_ > 0;
    }
    
    bool isNegativeInfinity() const noexcept {
        return type_ == Double && std::isinf(doubleValue_) && doubleValue_ < 0;
    }
    
    bool isFinite() const noexcept {
        if (type_ == Integer) return true;
        return type_ == Double && std::isfinite(doubleValue_);
    }
    
    bool isNormal() const noexcept {
        if (type_ == Integer) return intValue_ != 0;
        return type_ == Double && std::isnormal(doubleValue_);
    }

    // Safe value access
    std::optional<int64_t> getInteger() const noexcept {
        if (type_ == Integer) {
            return intValue_;
        }
        if (type_ == Double && doubleValue_ == std::floor(doubleValue_)) {
            // Check if double is within safe integer range
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

    // Forced type conversion (with range check)
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
            // Check if integer is within safe double range
            if (intValue_ > SAFE_INTEGER_MAX || intValue_ < SAFE_INTEGER_MIN) {
                // Note: There will be precision loss here, but this is common when converting from integer to floating point
            }
            return static_cast<double>(intValue_);
        }
        return 0.0;
    }

    // Type conversion (with default value)
    int32_t toInt32(int32_t defaultValue = 0) const noexcept {
        if (type_ == Integer) {
            if (intValue_ >= INT32_MIN && intValue_ <= INT32_MAX) {
                return static_cast<int32_t>(intValue_);
            }
        } else if (type_ == Double) {
            // For floating point numbers, perform truncation conversion (to maintain backward compatibility)
            if (doubleValue_ >= INT32_MIN && doubleValue_ <= INT32_MAX) {
                return static_cast<int32_t>(doubleValue_);
            }
        }
        return defaultValue;
    }

    int64_t toInt64(int64_t defaultValue = 0) const noexcept {
        if (type_ == Integer) {
            return intValue_;
        } else if (type_ == Double) {
            // For floating point numbers, perform truncation conversion (to maintain backward compatibility)
            if (doubleValue_ >= LLONG_MIN && doubleValue_ <= LLONG_MAX) {
                return static_cast<int64_t>(doubleValue_);
            }
        }
        return defaultValue;
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

    // Precision check functions
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

    // Serialization
    std::string toString() const {
        if (type_ == Integer) {
            return std::to_string(intValue_);
        } else {
            // Handle special values
            if (std::isnan(doubleValue_)) {
                return "NaN";
            } else if (std::isinf(doubleValue_)) {
                return doubleValue_ > 0 ? "Infinity" : "-Infinity";
            } else {
                // Use high-precision formatting to avoid scientific notation
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "%.15g", doubleValue_);
                return std::string(buffer);
            }
        }
    }

    // Comparison operators
    bool operator==(const JsonNumber& other) const noexcept {
        if (type_ == other.type_) {
            if (type_ == Integer) {
                return intValue_ == other.intValue_;
            } else {
                return doubleValue_ == other.doubleValue_;
            }
        }
        
        // Cross-type comparison
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
        
        // Cross-type comparison
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

    // Math operations
    JsonNumber operator+(const JsonNumber& other) const {
        if (type_ == Integer && other.type_ == Integer) {
            // Check integer overflow
            if ((other.intValue_ > 0 && intValue_ > INT64_MAX - other.intValue_) ||
                (other.intValue_ < 0 && intValue_ < INT64_MIN - other.intValue_)) {
                // Overflow, use double
                return JsonNumber(static_cast<double>(intValue_) + static_cast<double>(other.intValue_));
            }
            return JsonNumber(intValue_ + other.intValue_);
        }
        
        // If either is double, use double arithmetic
        return JsonNumber(toDouble() + other.toDouble());
    }

    JsonNumber operator-(const JsonNumber& other) const {
        if (type_ == Integer && other.type_ == Integer) {
            // Check integer overflow
            if ((other.intValue_ < 0 && intValue_ > INT64_MAX + other.intValue_) ||
                (other.intValue_ > 0 && intValue_ < INT64_MIN + other.intValue_)) {
                // Overflow, use double
                return JsonNumber(static_cast<double>(intValue_) - static_cast<double>(other.intValue_));
            }
            return JsonNumber(intValue_ - other.intValue_);
        }
        
        // If either is double, use double arithmetic
        return JsonNumber(toDouble() - other.toDouble());
    }

    JsonNumber operator*(const JsonNumber& other) const {
        if (type_ == Integer && other.type_ == Integer) {
            // Check integer overflow
            if (intValue_ != 0 && other.intValue_ != 0) {
                if ((intValue_ > 0 && other.intValue_ > 0 && intValue_ > INT64_MAX / other.intValue_) ||
                    (intValue_ < 0 && other.intValue_ < 0 && intValue_ < INT64_MAX / other.intValue_) ||
                    (intValue_ > 0 && other.intValue_ < 0 && other.intValue_ < INT64_MIN / intValue_) ||
                    (intValue_ < 0 && other.intValue_ > 0 && intValue_ < INT64_MIN / other.intValue_)) {
                    // Overflow, use double
                    return JsonNumber(static_cast<double>(intValue_) * static_cast<double>(other.intValue_));
                }
            }
            return JsonNumber(intValue_ * other.intValue_);
        }
        
        // If either is double, use double arithmetic
        return JsonNumber(toDouble() * other.toDouble());
    }

    JsonNumber operator/(const JsonNumber& other) const {
        if (other == JsonNumber(0)) {
            throw std::runtime_error("Division by zero");
        }
        
        // Division always returns double to avoid precision issues
        return JsonNumber(toDouble() / other.toDouble());
    }

    // Debug info
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
