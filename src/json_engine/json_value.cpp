#include "json_value.h"
#include <cctype>
#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <sstream>

namespace JsonStruct {

void JsonValue::dumpImpl(std::ostream& os, const SerializeOptions& options, int currentIndent) const {
    visit([&](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        
        if constexpr (std::is_same_v<T, std::monostate>) {
            os << "null";
        } else if constexpr (std::is_same_v<T, bool>) {
            os << (value ? "true" : "false");
        } else if constexpr (std::is_same_v<T, JsonNumber>) {
            // Use JsonNumber's smart formatting
            if (value.isInteger()) {
                os << value.toInteger();
            } else if (options.allowSpecialNumbers && (value.isNaN() || value.isInfinity())) {
                // Serialize special values
                os << value.toString();
            } else if (value.isNaN() || value.isInfinity()) {
                // If special values are not allowed, convert to null
                os << "null";
            } else {
                // Regular floating point uses specified precision
                os << std::setprecision(options.maxPrecision) << value.toDouble();
            }
        } else if constexpr (std::is_same_v<T, std::string>) {
            os << "\"" << escapeString(value, options.escapeUnicode) << "\"";
        } else if constexpr (std::is_same_v<T, ArrayType>) {
            os << "[";
            bool compact = options.compactArrays || options.indent < 0;
            
            for (size_t i = 0; i < value.size(); ++i) {
                if (i > 0) os << ",";
                if (!compact) {
                    os << "\n" << std::string(currentIndent + options.indent, ' ');
                }
                value[i].dumpImpl(os, options, currentIndent + options.indent);
            }
            
            if (!compact && !value.empty()) {
                os << "\n" << std::string(currentIndent, ' ');
            }
            os << "]";
        } else if constexpr (std::is_same_v<T, ObjectType>) {
            os << "{";
            
            // Collect keys and optionally sort
            std::vector<std::string> keys;
            keys.reserve(value.size());
            for (const auto& [key, val] : value) {
                keys.push_back(key);
            }
            
            if (options.sortKeys) {
                std::sort(keys.begin(), keys.end());
            }
            
            bool first = true;
            for (const auto& key : keys) {
                if (!first) os << ",";
                first = false;
                
                if (options.indent >= 0) {
                    os << "\n" << std::string(currentIndent + options.indent, ' ');
                }
                
                os << "\"" << escapeString(key, options.escapeUnicode) << "\":";
                if (options.indent >= 0) os << " ";
                
                value.at(key).dumpImpl(os, options, currentIndent + options.indent);
            }
            
            if (options.indent >= 0 && !value.empty()) {
                os << "\n" << std::string(currentIndent, ' ');
            }
            os << "}";
        }
    });
}

std::string JsonValue::escapeString(std::string_view str, bool escapeUnicode) {
    std::string result;
    result.reserve(str.size() + str.size() / 4);
    
    for (auto it = str.begin(); it != str.end(); ++it) {
        unsigned char c = static_cast<unsigned char>(*it);
        
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (c < 0x20) {
                    // Control characters as Unicode escape
                    result += "\\u";
                    char hex[5];
                    std::sprintf(hex, "%04x", c);
                    result += hex;
                } else if (escapeUnicode && c >= 0x80) {
                    // Optional Unicode character escaping
                    result += "\\u";
                    char hex[5];
                    std::sprintf(hex, "%04x", c);
                    result += hex;
                } else {
                    result += static_cast<char>(c);
                }
                break;
        }
    }
    return result;
}

JsonValue JsonValue::parseValue(ParseContext& ctx) {
    skipWhitespace(ctx);
    
    if (!ctx.hasMore()) {
        throw std::runtime_error("Unexpected end of input at " + ctx.locationInfo());
    }
    
    ctx.validateDepth();

    char c = ctx.peek();
    switch (c) {
        case 'n': return parseNull(ctx);
        case 't': case 'f': return parseBool(ctx);
        case '"': return parseString(ctx);
        case '[': 
            if (ctx.options.allowRecovery) {
                return parseArrayWithRecovery(ctx);
            } else {
                return parseArray(ctx);
            }
        case '{': 
            if (ctx.options.allowRecovery) {
                return parseObjectWithRecovery(ctx);
            } else {
                return parseObject(ctx);
            }
        case '-': {
            // Check for "-Infinity"
            if (ctx.options.allowSpecialNumbers && ctx.peek(1) == 'I') {
                return parseSpecialNumber(ctx);
            }
            // Otherwise, treat as number
            return parseNumber(ctx);
        }
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return parseNumber(ctx);
        case 'N': case 'I': {
            // Try to parse special numbers (NaN, Infinity)
            if (ctx.options.allowSpecialNumbers) {
                return parseSpecialNumber(ctx);
            }
            // If special values are not allowed, fallthrough to default
        }
        default:
            if (ctx.options.allowRecovery) {
                // Error recovery mode: skip invalid character
                ctx.advance(c);
                return parseValue(ctx); // Recursively try to parse next character
            }
            throw std::runtime_error("Unexpected character '" + std::string(1, c) + 
                                    "' at " + ctx.locationInfo());
    }
}

void JsonValue::skipWhitespace(ParseContext& ctx) {
    while (ctx.hasMore()) {
        char c = ctx.peek();
        if (std::isspace(c)) {
            ctx.advance(c);
        } else if (ctx.options.allowComments) {
            if (c == '/' && ctx.peek(1) == '/') {
                // Single-line comment
                while (ctx.hasMore() && ctx.peek() != '\n') {
                    ctx.advance(ctx.peek());
                }
            } else if (c == '/' && ctx.peek(1) == '*') {
                // Multi-line comment
                ctx.advance(ctx.peek()); // skip '/'
                ctx.advance(ctx.peek()); // skip '*'
                while (ctx.hasMore()) {
                    if (ctx.peek() == '*' && ctx.peek(1) == '/') {
                        ctx.advance(ctx.peek()); // skip '*'
                        ctx.advance(ctx.peek()); // skip '/'
                        break;
                    }
                    ctx.advance(ctx.peek());
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

JsonValue JsonValue::parseNull(ParseContext& ctx) {
    constexpr std::string_view nullStr = "null";
    
    if (ctx.source.substr(ctx.position, nullStr.length()) == nullStr) {
        for (size_t i = 0; i < nullStr.length(); ++i) {
            ctx.advance(ctx.peek());
        }
        return JsonValue{};
    }
    throw std::runtime_error("Invalid null value at " + ctx.locationInfo());
}

JsonValue JsonValue::parseBool(ParseContext& ctx) {
    constexpr std::string_view trueStr = "true";
    constexpr std::string_view falseStr = "false";
    
    if (ctx.source.substr(ctx.position, trueStr.length()) == trueStr) {
        for (size_t i = 0; i < trueStr.length(); ++i) {
            ctx.advance(ctx.peek());
        }
        return JsonValue(true);
    } else if (ctx.source.substr(ctx.position, falseStr.length()) == falseStr) {
        for (size_t i = 0; i < falseStr.length(); ++i) {
            ctx.advance(ctx.peek());
        }
        return JsonValue(false);
    }
    throw std::runtime_error("Invalid boolean value at " + ctx.locationInfo());
}

JsonValue JsonValue::parseNumber(ParseContext& ctx) {
    size_t start = ctx.position;
    bool hasDecimal = false;
    bool hasExponent = false;
    
    // Sign
    if (ctx.peek() == '-') {
        ctx.advance(ctx.peek());
    }
    
    if (!ctx.hasMore() || !std::isdigit(ctx.peek())) {
        throw std::runtime_error("Invalid number format at " + ctx.locationInfo());
    }
    
    // Integer part
    if (ctx.peek() == '0') {
        ctx.advance(ctx.peek());
        // JSON standard: no numbers directly after 0
        if (ctx.hasMore() && std::isdigit(ctx.peek())) {
            if (ctx.options.strictMode) {
                throw std::runtime_error("Leading zeros not allowed at " + ctx.locationInfo());
            }
        }
    } else {
        while (ctx.hasMore() && std::isdigit(ctx.peek())) {
            ctx.advance(ctx.peek());
        }
    }
    
    // Decimal part
    if (ctx.hasMore() && ctx.peek() == '.') {
        hasDecimal = true;
        ctx.advance(ctx.peek());
        if (!ctx.hasMore() || !std::isdigit(ctx.peek())) {
            throw std::runtime_error("Invalid number format: expected digit after '.' at " + ctx.locationInfo());
        }
        while (ctx.hasMore() && std::isdigit(ctx.peek())) {
            ctx.advance(ctx.peek());
        }
    }
    
    // Exponent part
    if (ctx.hasMore() && (ctx.peek() == 'e' || ctx.peek() == 'E')) {
        hasExponent = true;
        ctx.advance(ctx.peek());
        if (ctx.hasMore() && (ctx.peek() == '+' || ctx.peek() == '-')) {
            ctx.advance(ctx.peek());
        }
        if (!ctx.hasMore() || !std::isdigit(ctx.peek())) {
            throw std::runtime_error("Invalid number format: expected digit in exponent at " + ctx.locationInfo());
        }
        while (ctx.hasMore() && std::isdigit(ctx.peek())) {
            ctx.advance(ctx.peek());
        }
    }
    
    auto numStr = ctx.source.substr(start, ctx.position - start);
    
    // Choose parsing method based on format
    if (hasDecimal || hasExponent) {
        // Floating point
        double result;
        auto [ptr, ec] = std::from_chars(numStr.data(), numStr.data() + numStr.size(), result);
        if (ec != std::errc{}) {
            throw std::runtime_error("Failed to parse number '" + std::string(numStr) + 
                                    "' at " + ctx.locationInfo());
        }
        return JsonValue(result);
    } else {
        // Integer: try to parse as int64_t first
        try {
            int64_t intResult;
            auto [ptr, ec] = std::from_chars(numStr.data(), numStr.data() + numStr.size(), intResult);
            if (ec == std::errc{}) {
                return JsonValue(static_cast<long long>(intResult));
            }
        } catch (...) {
            // Integer parsing failed, fallback to double
        }
        
        // Fallback to double parsing
        double result;
        auto [ptr, ec] = std::from_chars(numStr.data(), numStr.data() + numStr.size(), result);
        if (ec != std::errc{}) {
            throw std::runtime_error("Failed to parse number '" + std::string(numStr) + 
                                    "' at " + ctx.locationInfo());
        }
        return JsonValue(result);
    }
}

JsonValue JsonValue::parseSpecialNumber(ParseContext& ctx) {
    std::string_view source = ctx.source;
    size_t pos = ctx.position;
    
    // Try to match "NaN"
    if (pos + 3 <= source.length() && source.substr(pos, 3) == "NaN") {
        ctx.position += 3;
        ctx.column += 3;
        return JsonValue(JsonNumber::makeNaN());
    }
    
    // Try to match "Infinity"
    if (pos + 8 <= source.length() && source.substr(pos, 8) == "Infinity") {
        ctx.position += 8;
        ctx.column += 8;
        return JsonValue(JsonNumber::makeInfinity());
    }
    
    // Try to match "-Infinity"
    if (pos + 9 <= source.length() && source.substr(pos, 9) == "-Infinity") {
        ctx.position += 9;
        ctx.column += 9;
        return JsonValue(JsonNumber::makeNegativeInfinity());
    }
    
    // If none match, throw error
    throw std::runtime_error("Invalid special number at " + ctx.locationInfo());
}

std::string JsonValue::parseUnicodeEscape(std::string_view str, size_t& pos) {
    if (pos + 5 >= str.length()) {
        throw std::runtime_error("Invalid Unicode escape sequence: too short");
    }
    
    std::string hexStr(str.substr(pos + 2, 4));
    pos += 6; // Skip \uXXXX
    
    // Parse hexadecimal
    unsigned int codepoint;
    auto [ptr, ec] = std::from_chars(hexStr.data(), hexStr.data() + 4, codepoint, 16);
    if (ec != std::errc{}) {
        throw std::runtime_error("Invalid Unicode escape sequence: '" + hexStr + "'");
    }
    
    // Handle surrogate pairs for UTF-16
    if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
        // High surrogate, need following low surrogate
        if (pos + 5 >= str.length() || str.substr(pos, 2) != "\\u") {
            throw std::runtime_error("Invalid surrogate pair: missing low surrogate");
        }
        
        std::string lowHex(str.substr(pos + 2, 4));
        pos += 6;
        
        unsigned int lowSurrogate;
        auto [ptr2, ec2] = std::from_chars(lowHex.data(), lowHex.data() + 4, lowSurrogate, 16);
        if (ec2 != std::errc{} || lowSurrogate < 0xDC00 || lowSurrogate > 0xDFFF) {
            throw std::runtime_error("Invalid surrogate pair: invalid low surrogate");
        }
        
        // Merge surrogate pair
        codepoint = 0x10000 + ((codepoint & 0x3FF) << 10) + (lowSurrogate & 0x3FF);
    }
    
    // Convert to UTF-8
    std::string result;
    if (codepoint <= 0x7F) {
        result += static_cast<char>(codepoint);
    } else if (codepoint <= 0x7FF) {
        result += static_cast<char>(0xC0 | (codepoint >> 6));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0xFFFF) {
        result += static_cast<char>(0xE0 | (codepoint >> 12));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0x10FFFF) {
        result += static_cast<char>(0xF0 | (codepoint >> 18));
        result += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else {
        throw std::runtime_error("Invalid Unicode codepoint: " + std::to_string(codepoint));
    }
    
    return result;
}

JsonValue JsonValue::parseString(ParseContext& ctx) {
    if (ctx.peek() != '"') {
        throw std::runtime_error("Expected '\"' at " + ctx.locationInfo());
    }
    ctx.advance(ctx.peek()); // skip opening quote
    
    std::string result;
    while (ctx.hasMore() && ctx.peek() != '"') {
        if (ctx.peek() == '\\') {
            ctx.advance(ctx.peek()); // skip backslash
            if (!ctx.hasMore()) {
                throw std::runtime_error("Unexpected end of string at " + ctx.locationInfo());
            }
            
            char escape = ctx.peek();
            switch (escape) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case 'u': {
                    // Unicode escape support
                    size_t tempPos = ctx.position - 1; // Back to '\' position
                    try {
                        result += parseUnicodeEscape(ctx.source, tempPos);
                        ctx.position = tempPos;
                        continue; // Skip advance below
                    } catch (const std::exception&) {
                        throw std::runtime_error("Invalid Unicode escape at " + ctx.locationInfo());
                    }
                }
                default:
                    if (ctx.options.strictMode) {
                        throw std::runtime_error("Invalid escape sequence '\\" + 
                                               std::string(1, escape) + "' at " + ctx.locationInfo());
                    } else {
                        // Non-strict mode: keep unknown escape sequence
                        result += '\\';
                        result += escape;
                    }
                    break;
            }
            ctx.advance(ctx.peek());
        } else {
            char c = ctx.peek();
            
            // Validate UTF-8 (if enabled)
            if (ctx.options.validateUtf8 && static_cast<unsigned char>(c) >= 0x80) {
                // Simple UTF-8 validation
                // A more complete UTF-8 validation could be implemented here
            }
            
            result += c;
            ctx.advance(c);
        }
    }
    
    if (!ctx.hasMore()) {
        throw std::runtime_error("Unterminated string at " + ctx.locationInfo());
    }
    
    ctx.advance(ctx.peek()); // skip closing quote
    return JsonValue(std::move(result));
}

JsonValue JsonValue::parseArray(ParseContext& ctx) {
    if (ctx.peek() != '[') {
        throw std::runtime_error("Expected '[' at " + ctx.locationInfo());
    }
    ctx.advance(ctx.peek()); // skip '['
    ++ctx.depth;
    
    ArrayType arr;
    skipWhitespace(ctx);
    
    // Empty array
    if (ctx.hasMore() && ctx.peek() == ']') {
        ctx.advance(ctx.peek());
        --ctx.depth;
        return JsonValue(std::move(arr));
    }
    
    while (true) {
        arr.emplace_back(parseValue(ctx));
        skipWhitespace(ctx);
        
        if (!ctx.hasMore()) {
            throw std::runtime_error("Unterminated array at " + ctx.locationInfo());
        }
        
        char c = ctx.peek();
        if (c == ']') {
            ctx.advance(c);
            break;
        } else if (c == ',') {
            ctx.advance(c);
            skipWhitespace(ctx);
            
            // Handle trailing comma
            if (ctx.options.allowTrailingCommas && ctx.hasMore() && ctx.peek() == ']') {
                ctx.advance(ctx.peek());
                break;
            }
        } else {
            throw std::runtime_error("Expected ',' or ']' at " + ctx.locationInfo());
        }
    }

    --ctx.depth;
    return JsonValue(std::move(arr));
}

JsonValue JsonValue::parseObject(ParseContext& ctx) {
    if (ctx.peek() != '{') {
        throw std::runtime_error("Expected '{' at " + ctx.locationInfo());
    }
    ctx.advance(ctx.peek()); // skip '{'
    ++ctx.depth;

    ObjectType obj;
    skipWhitespace(ctx);

    // Empty object
    if (ctx.hasMore() && ctx.peek() == '}') {
        ctx.advance(ctx.peek());
        --ctx.depth;
        return JsonValue(std::move(obj));
    }

    while (true) {
        skipWhitespace(ctx);

        // Parse key
        if (!ctx.hasMore() || ctx.peek() != '"') {
            throw std::runtime_error("Expected string key at " + ctx.locationInfo());
        }

        auto keyValue = parseString(ctx);
        auto key = keyValue.toString();

        skipWhitespace(ctx);

        if (!ctx.hasMore() || ctx.peek() != ':') {
            throw std::runtime_error("Expected ':' at " + ctx.locationInfo());
        }
        ctx.advance(ctx.peek()); // skip ':'

        // Parse value
        auto value = parseValue(ctx);
        obj.emplace(std::move(key), std::move(value));

        skipWhitespace(ctx);

        if (!ctx.hasMore()) {
            throw std::runtime_error("Unterminated object at " + ctx.locationInfo());
        }

        char c = ctx.peek();
        if (c == '}') {
            ctx.advance(c);
            break;
        } else if (c == ',') {
            ctx.advance(c);
            skipWhitespace(ctx);

            // Handle trailing comma
            if (ctx.options.allowTrailingCommas && ctx.hasMore() && ctx.peek() == '}') {
                ctx.advance(ctx.peek());
                break;
            }
        } else {
            throw std::runtime_error("Expected ',' or '}' at " + ctx.locationInfo());
        }
    }

    --ctx.depth;
    return JsonValue(std::move(obj));
}

// Enhanced error recovery parsing functions
JsonValue JsonValue::parseObjectWithRecovery(ParseContext& ctx) {
    if (ctx.peek() != '{') {
        throw std::runtime_error("Expected '{' at " + ctx.locationInfo());
    }
    ctx.advance(ctx.peek()); // skip '{'
    ++ctx.depth;

    ObjectType obj;
    skipWhitespace(ctx);

    // Empty object
    if (ctx.hasMore() && ctx.peek() == '}') {
        ctx.advance(ctx.peek());
        --ctx.depth;
        return JsonValue(std::move(obj));
    }

    while (ctx.hasMore()) {
        skipWhitespace(ctx);

        if (!ctx.hasMore()) break;

        // Check for object end
        if (ctx.peek() == '}') {
            ctx.advance(ctx.peek());
            break;
        }

        try {
            // Try to parse key-value pair
            if (ctx.peek() != '"') {
                if (ctx.options.allowRecovery) {
                    // Skip until valid key or object end
                    while (ctx.hasMore() && ctx.peek() != '"' && ctx.peek() != '}') {
                        ctx.advance(ctx.peek());
                    }
                    if (!ctx.hasMore() || ctx.peek() == '}') continue;
                } else {
                    throw std::runtime_error("Expected string key at " + ctx.locationInfo());
                }
            }

            auto keyValue = parseString(ctx);
            auto key = keyValue.toString();

            skipWhitespace(ctx);

            if (!ctx.hasMore() || ctx.peek() != ':') {
                if (ctx.options.allowRecovery) {
                    // Skip until colon
                    while (ctx.hasMore() && ctx.peek() != ':' && ctx.peek() != '}' && ctx.peek() != ',') {
                        ctx.advance(ctx.peek());
                    }
                    if (!ctx.hasMore() || ctx.peek() != ':') continue;
                } else {
                    throw std::runtime_error("Expected ':' at " + ctx.locationInfo());
                }
            }
            ctx.advance(ctx.peek()); // skip ':'

            // Parse value
            auto value = parseValue(ctx);
            obj.emplace(std::move(key), std::move(value));

        } catch (const std::exception&) {
            if (!ctx.options.allowRecovery) {
                throw;
            }
            // In recovery mode, skip current key-value pair
            while (ctx.hasMore() && ctx.peek() != ',' && ctx.peek() != '}') {
                ctx.advance(ctx.peek());
            }
        }

        skipWhitespace(ctx);

        if (!ctx.hasMore()) break;

        char c = ctx.peek();
        if (c == '}') {
            ctx.advance(c);
            break;
        } else if (c == ',') {
            ctx.advance(c);
            skipWhitespace(ctx);

            // Handle trailing comma
            if (ctx.options.allowTrailingCommas && ctx.hasMore() && ctx.peek() == '}') {
                ctx.advance(ctx.peek());
                break;
            }
        } else if (ctx.options.allowRecovery) {
            // Skip unexpected character in recovery mode
            ctx.advance(c);
        } else {
            throw std::runtime_error("Expected ',' or '}' at " + ctx.locationInfo());
        }
    }

    --ctx.depth;
    return JsonValue(std::move(obj));
}

JsonValue JsonValue::parseArrayWithRecovery(ParseContext& ctx) {
    if (ctx.peek() != '[') {
        throw std::runtime_error("Expected '[' at " + ctx.locationInfo());
    }
    ctx.advance(ctx.peek()); // skip '['
    ++ctx.depth;
    
    ArrayType arr;
    skipWhitespace(ctx);
    
    // Empty array
    if (ctx.hasMore() && ctx.peek() == ']') {
        ctx.advance(ctx.peek());
        --ctx.depth;
        return JsonValue(std::move(arr));
    }
    
    while (ctx.hasMore()) {
        skipWhitespace(ctx);
        
        if (!ctx.hasMore()) break;
        
        // Check for array end
        if (ctx.peek() == ']') {
            ctx.advance(ctx.peek());
            break;
        }
        
        try {
            // Try to parse value
            arr.emplace_back(parseValue(ctx));
        } catch (const std::exception&) {
            if (!ctx.options.allowRecovery) {
                throw;
            }
            // In recovery mode, skip invalid value and add null as placeholder
            while (ctx.hasMore() && ctx.peek() != ',' && ctx.peek() != ']') {
                ctx.advance(ctx.peek());
            }
            arr.emplace_back(JsonValue(nullptr)); // Add null as placeholder
        }
        
        skipWhitespace(ctx);
        
        if (!ctx.hasMore()) break;
        
        char c = ctx.peek();
        if (c == ']') {
            ctx.advance(c);
            break;
        } else if (c == ',') {
            ctx.advance(c);
            skipWhitespace(ctx);
            
            // Handle trailing comma
            if (ctx.options.allowTrailingCommas && ctx.hasMore() && ctx.peek() == ']') {
                ctx.advance(ctx.peek());
                break;
            }
        } else if (ctx.options.allowRecovery) {
            // Skip unexpected character in recovery mode
            ctx.advance(c);
        } else {
            throw std::runtime_error("Expected ',' or ']' at " + ctx.locationInfo());
        }
    }
    
    --ctx.depth;
    return JsonValue(std::move(arr));
}

bool JsonValue::isValidUtf8(std::string_view str) {
    // Simplified UTF-8 validation
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        
        if (c <= 0x7F) {
            i += 1;
        } else if ((c >> 5) == 0x06) {
            if (i + 1 >= str.length()) return false;
            if ((static_cast<unsigned char>(str[i + 1]) >> 6) != 0x02) return false;
            i += 2;
        } else if ((c >> 4) == 0x0E) {
            if (i + 2 >= str.length()) return false;
            if ((static_cast<unsigned char>(str[i + 1]) >> 6) != 0x02) return false;
            if ((static_cast<unsigned char>(str[i + 2]) >> 6) != 0x02) return false;
            i += 3;
        } else if ((c >> 3) == 0x1E) {
            if (i + 3 >= str.length()) return false;
            if ((static_cast<unsigned char>(str[i + 1]) >> 6) != 0x02) return false;
            if ((static_cast<unsigned char>(str[i + 2]) >> 6) != 0x02) return false;
            if ((static_cast<unsigned char>(str[i + 3]) >> 6) != 0x02) return false;
            i += 4;
        } else {
            return false;
        }
    }
    return true;
}

// JSON Pointer implementation (RFC 6901)
std::vector<std::string> JsonValue::parseJsonPointer(std::string_view pointer) {
    std::vector<std::string> tokens;
    if (pointer.empty() || pointer == "/") {
        return tokens;
    }
    
    if (pointer[0] != '/') {
        throw std::runtime_error("JSON pointer must start with '/'");
    }
    
    size_t start = 1;
    for (size_t i = 1; i <= pointer.length(); ++i) {
        if (i == pointer.length() || pointer[i] == '/') {
            tokens.push_back(unescapeJsonPointer(pointer.substr(start, i - start)));
            start = i + 1;
        }
    }
    
    return tokens;
}

std::string JsonValue::unescapeJsonPointer(std::string_view token) {
    std::string result;
    for (size_t i = 0; i < token.length(); ++i) {
        if (token[i] == '~') {
            if (i + 1 < token.length()) {
                if (token[i + 1] == '1') {
                    result += '/';
                    ++i;
                } else if (token[i + 1] == '0') {
                    result += '~';
                    ++i;
                } else {
                    throw std::runtime_error("Invalid JSON pointer escape");
                }
            } else {
                throw std::runtime_error("Invalid JSON pointer escape");
            }
        } else {
            result += token[i];
        }
    }
    return result;
}

JsonValue& JsonValue::at(std::string_view jsonPointer) {
    auto tokens = parseJsonPointer(jsonPointer);
    JsonValue* current = this;
    
    for (const auto& token : tokens) {
        if (current->isArray()) {
            // Array index
            try {
                size_t index = std::stoull(token);
                current = &((*current)[index]);
            } catch (const std::exception&) {
                throw std::runtime_error("Invalid array index: " + token);
            }
        } else if (current->isObject()) {
            // Object key
            current = &((*current)[token]);
        } else {
            throw std::runtime_error("Cannot index into non-container type");
        }
    }
    
    return *current;
}

const JsonValue& JsonValue::at(std::string_view jsonPointer) const {
    return const_cast<JsonValue*>(this)->at(jsonPointer);
}

// Literal operator
namespace literals {
    JsonValue operator""_json(const char* str, size_t len) {
        return JsonValue::parse(std::string_view(str, len));
    }
}

// Enhanced JSONPath implementation
// Supports basic dot notation paths and array indices, like "$.store.book[0].title" or "$.arr[1:3]"
bool JsonValue::pathExists(const std::string& jsonpath_expression) const {
    if (jsonpath_expression.empty() || jsonpath_expression[0] != '$') {
        return false;
    }
    
    if (jsonpath_expression == "$") {
        return true;  // Root always exists
    }
    
    return selectFirst(jsonpath_expression) != nullptr;
}

const JsonValue* JsonValue::selectFirst(const std::string& jsonpath_expression) const {
    if (jsonpath_expression.empty() || jsonpath_expression[0] != '$') {
        return nullptr;
    }
    
    if (jsonpath_expression == "$") {
        return this;
    }
    
    // Parse path components
    std::string path = jsonpath_expression.substr(1);  // Remove '$'
    if (path.empty() || path[0] != '.') {
        return nullptr;
    }
    
    path = path.substr(1);  // Remove leading '.'
    if (path.empty()) {
        return nullptr;  // Path like "$." is invalid
    }
    
    const JsonValue* current = this;
    std::string::size_type pos = 0;
    
    while (pos < path.length()) {
        std::string::size_type nextDot = path.find('.', pos);
        std::string::size_type nextBracket = path.find('[', pos);
        
        if (nextBracket != std::string::npos && (nextDot == std::string::npos || nextBracket < nextDot)) {
            // Handle property followed by array index: prop[index]
            std::string property = path.substr(pos, nextBracket - pos);
            if (property.empty()) {
                return nullptr;
            }
            
            // Navigate to the property
            if (!current->isObject() || !current->contains(property)) {
                return nullptr;
            }
            current = &(*current)[property];
            
            // Parse the array index
            std::string::size_type closeBracket = path.find(']', nextBracket);
            if (closeBracket == std::string::npos) {
                return nullptr;  // Malformed bracket
            }
            
            std::string indexStr = path.substr(nextBracket + 1, closeBracket - nextBracket - 1);
            if (indexStr.empty()) {
                return nullptr;  // Empty brackets not supported
            }
            
            // Check for slice notation (e.g., "1:3")
            std::string::size_type colonPos = indexStr.find(':');
            if (colonPos != std::string::npos) {
                // Array slicing support
                std::string startStr = indexStr.substr(0, colonPos);
                std::string endStr = indexStr.substr(colonPos + 1);
                
                try {
                    int start = startStr.empty() ? 0 : std::stoi(startStr);
                    
                    if (!current->isArray()) {
                        return nullptr;
                    }
                    
                    const auto* arr = current->getArray();
                    int end = endStr.empty() ? static_cast<int>(arr->size()) : std::stoi(endStr);
                    
                    // Validate bounds
                    if (start < 0 || end < 0 || start >= static_cast<int>(arr->size()) || start >= end) {
                        return nullptr;
                    }
                    
                    // For selectFirst, return the first element in the slice
                    if (start < static_cast<int>(arr->size())) {
                        current = &(*arr)[start];
                    } else {
                        return nullptr;
                    }
                } catch (const std::exception&) {
                    return nullptr;  // Invalid slice format
                }
            } else {
                // Parse single index
                try {
                    int index = std::stoi(indexStr);
                    if (!current->isArray()) {
                        return nullptr;
                    }
                    
                    const auto* arr = current->getArray();
                    if (index < 0 || static_cast<size_t>(index) >= arr->size()) {
                        return nullptr;  // Index out of bounds
                    }
                    
                    current = &(*arr)[index];
                } catch (const std::exception&) {
                    return nullptr;  // Invalid index format
                }
            }
            
            pos = closeBracket + 1;
            if (pos < path.length() && path[pos] == '.') {
                pos++;  // Skip dot after bracket
            }
        } else if (nextDot != std::string::npos) {
            // Handle simple property access
            std::string property = path.substr(pos, nextDot - pos);
            if (property.empty()) {
                return nullptr;
            }
            
            if (!current->isObject() || !current->contains(property)) {
                return nullptr;
            }
            current = &(*current)[property];
            pos = nextDot + 1;
        } else {
            // Handle final property
            std::string property = path.substr(pos);
            if (property.empty()) {
                return nullptr;
            }
            
            if (!current->isObject() || !current->contains(property)) {
                return nullptr;
            }
            current = &(*current)[property];
            break;
        }
    }
    
    return current;
}

// Enhanced JSONPath support - multi-value query implementation
std::vector<const JsonValue*> JsonValue::selectAll(const std::string& jsonpath_expression) const {
    std::vector<const JsonValue*> results;
    
    if (jsonpath_expression.empty() || jsonpath_expression[0] != '$') {
        return results;
    }
    
    if (jsonpath_expression == "$") {
        results.push_back(this);
        return results;
    }
    
    // Handle special patterns - check for array wildcard [*] first
    if (jsonpath_expression.find("[*]") != std::string::npos) {
        return selectAllWithWildcard(jsonpath_expression);
    }
    
    // Handle other wildcard patterns
    if (jsonpath_expression.find("*") != std::string::npos) {
        return selectAllWithWildcard(jsonpath_expression);
    }
    
    if (jsonpath_expression.find("..") != std::string::npos) {
        return selectAllWithRecursiveDescent(jsonpath_expression);
    }
    
    // Handle array slicing
    if (jsonpath_expression.find("[") != std::string::npos && jsonpath_expression.find(":") != std::string::npos) {
        return selectAllWithSlicing(jsonpath_expression);
    }
    
    // For simple paths, use selectFirst
    const auto* result = selectFirst(jsonpath_expression);
    if (result != nullptr) {
        results.push_back(result);
    }
    
    return results;
}

std::vector<JsonValue> JsonValue::selectValues(const std::string& jsonpath_expression) const {
    std::vector<JsonValue> results;
    auto pointers = selectAll(jsonpath_expression);
    
    results.reserve(pointers.size());
    for (const auto* ptr : pointers) {
        if (ptr != nullptr) {
            results.push_back(*ptr);
        }
    }
    
    return results;
}

// Wildcard support private helper method
std::vector<const JsonValue*> JsonValue::selectAllWithWildcard(const std::string& jsonpath_expression) const {
    std::vector<const JsonValue*> results;
    
    // Simplified implementation: support $.* (root-level property wildcard)
    if (jsonpath_expression == "$.*") {
        if (isObject()) {
            const auto* obj = getObject();
            for (const auto& pair : *obj) {
                results.push_back(&pair.second);
            }
        } else if (isArray()) {
            const auto* arr = getArray();
            for (const auto& item : *arr) {
                results.push_back(&item);
            }
        }
        return results;
    }
    
    // Support $.prop[*].subprop pattern
    std::string::size_type arrayWildcardPos = jsonpath_expression.find("[*]");
    if (arrayWildcardPos != std::string::npos) {
        std::string basePath = jsonpath_expression.substr(0, arrayWildcardPos);
        std::string afterWildcard = jsonpath_expression.substr(arrayWildcardPos + 3); // Skip "[*]"
        
        const auto* baseNode = selectFirst(basePath);
        
        if (baseNode != nullptr && baseNode->isArray()) {
            const auto* arr = baseNode->getArray();
            for (const auto& item : *arr) {
                // Apply the rest of the path to each array element
                if (!afterWildcard.empty()) {
                    // Remove leading dot if present
                    if (afterWildcard[0] == '.') {
                        afterWildcard = afterWildcard.substr(1);
                    }
                    
                    if (!afterWildcard.empty()) {
                        // Navigate to the property in each array element
                        if (item.isObject() && item.contains(afterWildcard)) {
                            results.push_back(&item[afterWildcard]);
                        }
                    }
                } else {
                    // Just return all array elements
                    results.push_back(&item);
                }
            }
        }
        return results;
    }
    
    // Support $.prop.* pattern
    std::string::size_type wildcardPos = jsonpath_expression.find(".*");
    if (wildcardPos != std::string::npos) {
        std::string basePath = jsonpath_expression.substr(0, wildcardPos);
        const auto* baseNode = selectFirst(basePath);
        
        if (baseNode != nullptr) {
            if (baseNode->isObject()) {
                const auto* obj = baseNode->getObject();
                for (const auto& pair : *obj) {
                    results.push_back(&pair.second);
                }
            } else if (baseNode->isArray()) {
                const auto* arr = baseNode->getArray();
                for (const auto& item : *arr) {
                    results.push_back(&item);
                }
            }
        }
    }
    
    return results;
}

// Recursive descent support private helper method
std::vector<const JsonValue*> JsonValue::selectAllWithRecursiveDescent(const std::string& jsonpath_expression) const {
    std::vector<const JsonValue*> results;
    
    // Support $..prop pattern (recursively find all properties named prop)
    std::string::size_type recursivePos = jsonpath_expression.find("..");
    if (recursivePos != std::string::npos) {
        std::string targetProp = jsonpath_expression.substr(recursivePos + 2);
        
        // If target property name starts with '.', remove it
        if (!targetProp.empty() && targetProp[0] == '.') {
            targetProp = targetProp.substr(1);
        }
        
        // Recursive search
        recursiveSearch(targetProp, results);
    }
    
    return results;
}

// Array slicing support private helper method
std::vector<const JsonValue*> JsonValue::selectAllWithSlicing(const std::string& jsonpath_expression) const {
    std::vector<const JsonValue*> results;
    
    // Parse path until array slicing
    std::string::size_type bracketPos = jsonpath_expression.find('[');
    if (bracketPos == std::string::npos) {
        return results;
    }
    
    std::string basePath = jsonpath_expression.substr(0, bracketPos);
    const auto* arrayNode = selectFirst(basePath);
    
    if (arrayNode == nullptr || !arrayNode->isArray()) {
        return results;
    }
    
    // Parse slice expression
    std::string::size_type closeBracket = jsonpath_expression.find(']', bracketPos);
    if (closeBracket == std::string::npos) {
        return results;
    }
    
    std::string sliceExpr = jsonpath_expression.substr(bracketPos + 1, closeBracket - bracketPos - 1);
    std::string::size_type colonPos = sliceExpr.find(':');
    
    if (colonPos != std::string::npos) {
        try {
            std::string startStr = sliceExpr.substr(0, colonPos);
            std::string endStr = sliceExpr.substr(colonPos + 1);
            
            const auto* arr = arrayNode->getArray();
            int start = startStr.empty() ? 0 : std::stoi(startStr);
            int end = endStr.empty() ? static_cast<int>(arr->size()) : std::stoi(endStr);
            
            // Ensure range is valid
            start = std::max(0, std::min(start, static_cast<int>(arr->size())));
            end = std::max(start, std::min(end, static_cast<int>(arr->size())));
            
            // Add all elements in slice range
            for (int i = start; i < end; ++i) {
                results.push_back(&(*arr)[i]);
            }
        } catch (const std::exception&) {
            // Ignore parsing errors
        }
    }
    
    return results;
}

// Recursive search helper method
void JsonValue::recursiveSearch(const std::string& targetProp, std::vector<const JsonValue*>& results) const {
    if (isObject()) {
        const auto* obj = getObject();
        
        // Check current level for target property
        auto it = obj->find(targetProp);
        if (it != obj->end()) {
            results.push_back(&it->second);
        }
        
        // Recursive search all child objects
        for (const auto& pair : *obj) {
            pair.second.recursiveSearch(targetProp, results);
        }
    } else if (isArray()) {
        const auto* arr = getArray();
        
        // Recursive search all array elements
        for (const auto& item : *arr) {
            item.recursiveSearch(targetProp, results);
        }
    }
}

std::ostream& operator<<(std::ostream& os, const JsonValue& value) {
    os << value.dump();
    return os;
}

std::istream& operator>>(std::istream& is, JsonValue& value) {
    std::string json((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    value = JsonValue::parse(json);
    return is;
}

} // namespace JsonStruct
