#include "json_value.h"
#include "json_filter.h"
#include "json_query_generator.h"
#include "json_error.h"
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
            // os << "null";
            os << "";
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

bool JsonValue::utf8Check(ParseContext& ctx, std::string_view str)
{
    // Validate UTF-8 (if enabled)
    if (ctx.options.validateUtf8) {
        std::string_view toCheck(str);
        if (!isValidUtf8(toCheck)) {
            ctx.setError(JsonErrc::Utf8Error, "Invalid UTF-8 sequence at " + ctx.locationInfo());
            return false;
        }
    }

    return true;
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
                    unsigned int codepoint = 0;
                    size_t len = 1;
                    if ((c & 0xF8) == 0xF0 && (it + 3 < str.end())) {
                        codepoint = ((c & 0x07) << 18) |
                            ((static_cast<unsigned char>(*(it + 1)) & 0x3F) << 12) |
                            ((static_cast<unsigned char>(*(it + 2)) & 0x3F) << 6) |
                            (static_cast<unsigned char>(*(it + 3)) & 0x3F);
                        len = 4;
                    }
                    else if ((c & 0xF0) == 0xE0 && (it + 2 < str.end())) {
                        codepoint = ((c & 0x0F) << 12) |
                            ((static_cast<unsigned char>(*(it + 1)) & 0x3F) << 6) |
                            (static_cast<unsigned char>(*(it + 2)) & 0x3F);
                        len = 3;
                    }
                    else if ((c & 0xE0) == 0xC0 && (it + 1 < str.end())) {
                        codepoint = ((c & 0x1F) << 6) | (static_cast<unsigned char>(*(it + 1)) & 0x3F);
                        len = 2;
                    }
                    else {
                        codepoint = c;
                        len = 1;
                    }
                    if (codepoint <= 0xFFFF) {
                        char hex[7];
                        sprintf_s(hex, "\\u%04x", codepoint);
                        result += hex;
                    }
                    else {
                        // Surrogate pair
                        unsigned int high = 0xD800 + ((codepoint - 0x10000) >> 10);
                        unsigned int low = 0xDC00 + ((codepoint - 0x10000) & 0x3FF);
                        char hex[13];
                        sprintf_s(hex, "\\u%04x\\u%04x", high, low);
                        result += hex;
                    }
                    it += (len - 1);
                } else {
                    result += static_cast<char>(c);
                }
                break;
        }
    }
    return result;
}

JsonValue JsonValue::parseValue(ParseContext& ctx) {
    if (ctx.hasError()) return JsonValue{};
    
    skipWhitespace(ctx);
    if (ctx.hasError()) return JsonValue{};
    
    if (!ctx.hasMore()) {
        ctx.setError(JsonErrc::UnexpectedEnd, "Unexpected end of input at " + ctx.locationInfo());
        return JsonValue{};
    }
    
    ctx.validateDepth();
    if (ctx.hasError()) return JsonValue{};

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
            [[fallthrough]];
        }
        default:
            if (ctx.options.allowRecovery) {
                // Error recovery mode: skip invalid character
                ctx.advance(c);
                return parseValue(ctx); // Recursively try to parse next character
            }
            ctx.setError(JsonErrc::UnexpectedCharacter, 
                "Unexpected character '" + std::string(1, c) + "' at " + ctx.locationInfo());
            return JsonValue{};
    }
}

void JsonValue::skipWhitespace(ParseContext& ctx) {
    while (ctx.hasMore()) {
        char c = ctx.peek();
        if (std::isspace(static_cast<unsigned char>(c))) {
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
    if (ctx.hasError()) return JsonValue{};
    
    constexpr std::string_view nullStr = "null";
    
    if (ctx.source.substr(ctx.position, nullStr.length()) == nullStr) {
        for (size_t i = 0; i < nullStr.length(); ++i) {
            ctx.advance(ctx.peek());
        }
        return JsonValue{};
    }
    ctx.setError(JsonErrc::ParseError, "Invalid null value at " + ctx.locationInfo());
    return JsonValue{};
}

JsonValue JsonValue::parseBool(ParseContext& ctx) {
    if (ctx.hasError()) return JsonValue{};
    
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
    ctx.setError(JsonErrc::UnexpectedCharacter, "Invalid boolean value at " + ctx.locationInfo());
    return JsonValue{};
}

JsonValue JsonValue::parseNumber(ParseContext& ctx) {
    if (ctx.hasError()) return JsonValue{};
    
    size_t start = ctx.position;
    bool hasDecimal = false;
    bool hasExponent = false;
    
    // Sign
    if (ctx.peek() == '-') {
        ctx.advance(ctx.peek());
    }
    
    if (!ctx.hasMore() || !std::isdigit(ctx.peek())) {
        ctx.setError(JsonErrc::ParseError, "Invalid number format at " + ctx.locationInfo());
        return JsonValue{};
    }
    
    // Integer part
    if (ctx.peek() == '0') {
        ctx.advance(ctx.peek());
        // JSON standard: no numbers directly after 0
        if (ctx.hasMore() && std::isdigit(ctx.peek())) {
            if (ctx.options.strictMode) {
                ctx.setError(JsonErrc::ParseError, "Leading zeros not allowed at " + ctx.locationInfo());
                return JsonValue{};
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
            ctx.setError(JsonErrc::ParseError, "Invalid number format: expected digit after '.' at " + ctx.locationInfo());
            return JsonValue{};
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
            ctx.setError(JsonErrc::ParseError, "Invalid number format: expected digit in exponent at " + ctx.locationInfo());
            return JsonValue{};
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
            ctx.setError(JsonErrc::ParseError, "Failed to parse number '" + std::string(numStr) + "' at " + ctx.locationInfo());
            return JsonValue{};
        }
        return JsonValue(result);
    } else {
        // Integer: try to parse as int64_t first
        int64_t intResult;
        auto [ptr, ec] = std::from_chars(numStr.data(), numStr.data() + numStr.size(), intResult);
        if (ec == std::errc{}) {
            return JsonValue(static_cast<long long>(intResult));
        }
        
        // Fallback to double parsing
        double result;
        auto [ptr2, ec2] = std::from_chars(numStr.data(), numStr.data() + numStr.size(), result);
        if (ec2 != std::errc{}) {
            ctx.setError(JsonErrc::ParseError, "Failed to parse number '" + std::string(numStr) + "' at " + ctx.locationInfo());
            return JsonValue{};
        }
        return JsonValue(result);
    }
}

JsonValue JsonValue::parseSpecialNumber(ParseContext& ctx) {
    if (ctx.hasError()) return JsonValue{};
    
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
    
    // If none match, set error
    ctx.setError(JsonErrc::ParseError, "Invalid special number at " + ctx.locationInfo());
    return JsonValue{};
}

std::string JsonValue::parseUnicodeEscape(std::string_view str, size_t& pos) {
    if (pos + 6 >= str.length()) {
        throw std::runtime_error("Invalid Unicode escape sequence: too short");
    }
    
    std::string hexStr(str.substr(pos + 2, 4));
    
    // Parse hexadecimal
    unsigned int codepoint;
    auto [ptr, ec] = std::from_chars(hexStr.data(), hexStr.data() + 4, codepoint, 16);
    if (ec != std::errc{} || ptr != hexStr.data() + 4) {
        throw std::runtime_error("Invalid Unicode escape sequence: '" + hexStr + "'");
    }
    pos += 6; // Skip \uXXXX

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

std::error_code JsonValue::parseUnicodeEscapeSafe(std::string_view str, size_t& pos, std::string& result, std::string& errMsg) {
    if (pos + 6 > str.length()) {
        errMsg = "Invalid Unicode escape sequence: too short";
        return make_error_code(JsonErrc::ParseError);
    }
    
    std::string hexStr(str.substr(pos + 2, 4));
    pos += 6; // Skip \uXXXX
    
    // Parse hexadecimal
    unsigned int codepoint;
    auto [ptr, ec] = std::from_chars(hexStr.data(), hexStr.data() + 4, codepoint, 16);
    if (ec != std::errc{} || ptr != hexStr.data() + 4) {
        errMsg = "Invalid Unicode escape sequence: '" + hexStr + "'";
        return make_error_code(JsonErrc::ParseError);
    }
    
    // Handle surrogate pairs for UTF-16
    if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
        // High surrogate, need following low surrogate
        if (pos + 5 >= str.length() || str.substr(pos, 2) != "\\u") {
            errMsg = "Invalid surrogate pair: missing low surrogate";
            return make_error_code(JsonErrc::ParseError);
        }
        
        std::string lowHex(str.substr(pos + 2, 4));
        pos += 6;
        
        unsigned int lowSurrogate;
        auto [ptr2, ec2] = std::from_chars(lowHex.data(), lowHex.data() + 4, lowSurrogate, 16);
        if (ec2 != std::errc{} || ptr2 != lowHex.data() + 4 || lowSurrogate < 0xDC00 || lowSurrogate > 0xDFFF) {
            errMsg = "Invalid surrogate pair: invalid low surrogate";
            return make_error_code(JsonErrc::ParseError);
        }
        
        // Merge surrogate pair
        codepoint = 0x10000 + ((codepoint & 0x3FF) << 10) + (lowSurrogate & 0x3FF);
    }
    
    // Convert to UTF-8
    result.clear();
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
        errMsg = "Invalid Unicode codepoint: " + std::to_string(codepoint);
        return make_error_code(JsonErrc::ParseError);
    }
    
    return make_error_code(JsonErrc::Success);
}

JsonValue JsonValue::parseString(ParseContext& ctx) {
    if (ctx.hasError()) return JsonValue{};
    
    if (ctx.peek() != '"') {
        ctx.setError(JsonErrc::UnexpectedCharacter, "Expected '\"' at " + ctx.locationInfo());
        return JsonValue{};
    }
    ctx.advance(ctx.peek()); // skip opening quote
    
    std::string result;
    while (ctx.hasMore()) {
        char c = ctx.peek();
        if (c == '"') {
            break;
        }
        if (c == '\\') {
            ctx.advance(c); // skip backslash
            if (!ctx.hasMore()) {
                ctx.setError(JsonErrc::UnexpectedEnd, "Unexpected end of string at " + ctx.locationInfo());
                return JsonValue{};
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
                    size_t tempPos = ctx.position - 1; // Back to '\\' position
                    try {
                        result += parseUnicodeEscape(ctx.source, tempPos);
                        ctx.position = tempPos;
                        continue; // Skip advance below
                    } catch (const std::exception&) {
                        ctx.setError(JsonErrc::Utf8Error, "Invalid Unicode escape at " + ctx.locationInfo());
                        return JsonValue{};
                    }
                }
                default:
                    if (ctx.options.strictMode) {
                        ctx.setError(JsonErrc::ParseError, "Invalid escape sequence '\\" + std::string(1, escape) + "' at " + ctx.locationInfo());
                        return JsonValue{};
                    } else {
                        // Non-strict mode: keep unknown escape sequence
                        result += '\\';
                        result += escape;
                    }
                    break;
            }
            ctx.advance(ctx.peek());
        } else {
            // support multi-line strings: allow actual newline characters in string content
            if (c == '\n' || c == '\r') {
                if(ctx.options.strictMode) {
                    ctx.setError(JsonErrc::ParseError, "Strict Mode not allowed multi line string at " + ctx.locationInfo());
                    return JsonValue{};
				}
                result += c;
                ctx.advance(c);
                continue;
            }
            result += c;
            ctx.advance(c);
        }
    }
    
    if (!ctx.hasMore()) {
        ctx.setError(JsonErrc::UnexpectedEnd, "Unterminated string at " + ctx.locationInfo());
        return JsonValue{};
    }
    
    if (!utf8Check(ctx, result))
        return JsonValue{};

    ctx.advance(ctx.peek()); // skip closing quote
    return JsonValue(std::move(result));
}

JsonValue JsonValue::parseArray(ParseContext& ctx) {
    if (ctx.hasError()) return JsonValue{};
    
    if (ctx.peek() != '[') {
        ctx.setError(JsonErrc::UnexpectedCharacter, "Expected '[' at " + ctx.locationInfo());
        return JsonValue{};
    }
    ctx.advance(ctx.peek()); // skip '['
    ++ctx.depth;
    
    ArrayType arr;
    skipWhitespace(ctx);
    if (ctx.hasError()) {
        --ctx.depth;
        return JsonValue{};
    }
    
    // Empty array
    if (ctx.hasMore() && ctx.peek() == ']') {
        ctx.advance(ctx.peek());
        --ctx.depth;
        return JsonValue(std::move(arr));
    }
    
    while (true) {
        arr.emplace_back(parseValue(ctx));
        if (ctx.hasError()) {
            --ctx.depth;
            return JsonValue{};
        }
        
        skipWhitespace(ctx);
        if (ctx.hasError()) {
            --ctx.depth;
            return JsonValue{};
        }
        
        if (!ctx.hasMore()) {
            ctx.setError(JsonErrc::UnexpectedEnd, "Unterminated array at " + ctx.locationInfo());
            --ctx.depth;
            return JsonValue{};
        }
        
        char c = ctx.peek();
        if (c == ']') {
            ctx.advance(c);
            break;
        } else if (c == ',') {
            ctx.advance(c);
            skipWhitespace(ctx);
            if (ctx.hasError()) {
                --ctx.depth;
                return JsonValue{};
            }
            
            // Handle trailing comma
            if (ctx.options.allowTrailingCommas) {
                while (ctx.hasMore() && ctx.peek() == ',' && ctx.peek() != ']') {
                    ctx.advance(ctx.peek());
                }
                if (ctx.hasMore() && ctx.peek() == ']') {
                    ctx.advance(ctx.peek());
                    break;
                }
            }
        } else {
            ctx.setError(JsonErrc::UnexpectedCharacter, "Expected ',' or ']' at " + ctx.locationInfo());
            --ctx.depth;
            return JsonValue{};
        }
    }

    --ctx.depth;
    return JsonValue(std::move(arr));
}

JsonValue JsonValue::parseObject(ParseContext& ctx) {
    if (ctx.hasError()) return JsonValue{};
    
    if (ctx.peek() != '{') {
        ctx.setError(JsonErrc::UnexpectedCharacter, "Expected '{' at " + ctx.locationInfo());
        return JsonValue{};
    }
    ctx.advance(ctx.peek()); // skip '{'
    ++ctx.depth;

    ObjectType obj;
    skipWhitespace(ctx);
    // if (ctx.hasError()) {
    //     --ctx.depth;
    //     return JsonValue{};
    // }

    // Empty object
    if (ctx.hasMore() && ctx.peek() == '}') {
        ctx.advance(ctx.peek());
        --ctx.depth;
        return JsonValue(std::move(obj));
    }

    while (true) {
        skipWhitespace(ctx);
        if (ctx.hasError()) {
            --ctx.depth;
            return JsonValue{};
        }

        // Parse key
        if (!ctx.hasMore() || ctx.peek() != '"') {
            ctx.setError(JsonErrc::UnexpectedCharacter, "Expected string key at " + ctx.locationInfo());
            --ctx.depth;
            return JsonValue{};
        }

        auto keyValue = parseString(ctx);
        if (ctx.hasError()) {
            --ctx.depth;
            return JsonValue{};
        }
        auto key = keyValue.toString();

        skipWhitespace(ctx);
        if (ctx.hasError()) {
            --ctx.depth;
            return JsonValue{};
        }

        if (!ctx.hasMore() || ctx.peek() != ':') {
            ctx.setError(JsonErrc::UnexpectedCharacter, "Expected ':' at " + ctx.locationInfo());
            --ctx.depth;
            return JsonValue{};
        }
        ctx.advance(ctx.peek()); // skip ':'

        // Parse value
        auto value = parseValue(ctx);
        if (ctx.hasError()) {
            --ctx.depth;
            return JsonValue{};
        }
        obj.emplace(std::move(key), std::move(value));

        skipWhitespace(ctx);
        if (ctx.hasError()) {
            --ctx.depth;
            return JsonValue{};
        }

        if (!ctx.hasMore()) {
            ctx.setError(JsonErrc::UnexpectedEnd, "Unterminated object at " + ctx.locationInfo());
            --ctx.depth;
            return JsonValue{};
        }

        char c = ctx.peek();
        if (c == '}') {
            ctx.advance(c);
            break;
        } else if (c == ',') {
            ctx.advance(c);
            skipWhitespace(ctx);
            if (ctx.hasError()) {
                --ctx.depth;
                return JsonValue{};
            }

            // Handle trailing comma
            if (ctx.options.allowTrailingCommas) {
                while (ctx.hasMore() && ctx.peek() == ',' && ctx.peek() != '}') {
                    ctx.advance(ctx.peek());
                }
                if (ctx.hasMore() && ctx.peek() == '}') {
                    ctx.advance(ctx.peek());
                    break;
                }
            }
        } else {
            ctx.setError(JsonErrc::UnexpectedCharacter, "Expected ',' or '}' at " + ctx.locationInfo());
            --ctx.depth;
            return JsonValue{};
        }
    }

    --ctx.depth;
    return JsonValue(std::move(obj));
}

// Enhanced error recovery parsing functions
JsonValue JsonValue::parseObjectWithRecovery(ParseContext& ctx) {
    if(ctx.hasError()) return JsonValue{};

    if (ctx.peek() != '{') {
        ctx.setError(JsonErrc::UnexpectedCharacter, "Expected '{' at " + ctx.locationInfo());
        return JsonValue{};
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
                // Skip until valid key or object end
                while (ctx.hasMore() && ctx.peek() != '"' && ctx.peek() != '}') {
                    ctx.advance(ctx.peek());
                }
                if (!ctx.hasMore() || ctx.peek() == '}') continue;
            }

            auto keyValue = parseString(ctx);
            auto key = keyValue.toString();

            skipWhitespace(ctx);

            if (!ctx.hasMore() || ctx.peek() != ':') {
                // Skip until colon
                while (ctx.hasMore() && ctx.peek() != ':' && ctx.peek() != '}' && ctx.peek() != ',') {
                    ctx.advance(ctx.peek());
                }
                if (!ctx.hasMore() || ctx.peek() != ':') continue;
            }
            ctx.advance(ctx.peek()); // skip ':'

            // Parse value
            auto value = parseValue(ctx);
            obj.emplace(std::move(key), std::move(value));

        } catch (const std::exception&) {
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
            if (ctx.options.allowTrailingCommas) {
                while (ctx.hasMore() && ctx.peek() == ',' && ctx.peek() != '}') {
                    ctx.advance(ctx.peek());
                }
                if (ctx.hasMore() && ctx.peek() == '}') {
                    ctx.advance(ctx.peek());
                    break;
                }
            }
        } else {
            // Skip unexpected character in recovery mode
            ctx.advance(c);
        }
    }

    --ctx.depth;
    return JsonValue(std::move(obj));
}

JsonValue JsonValue::parseArrayWithRecovery(ParseContext& ctx) {
    if (ctx.hasError()) return JsonValue{};
    // Check for '['
    if (ctx.peek() != '[') {
        ctx.setError(JsonErrc::UnexpectedCharacter, "Expected '[' at " + ctx.locationInfo());
        return JsonValue{};
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
            if (ctx.options.allowTrailingCommas) {
                while(ctx.hasMore() && ctx.peek() == ',' && ctx.peek() != ']') {
                    ctx.advance(ctx.peek());
				}
                if(ctx.hasMore() && ctx.peek() == ']') {
                    ctx.advance(ctx.peek());
                    break;
				}
            }
        } else {
            // Skip unexpected character in recovery mode
            ctx.advance(c);
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

std::error_code JsonValue::parseJsonPointerSafe(std::string_view pointer, std::vector<std::string>& tokens, std::string& errMsg) {
    tokens.clear();
    if (pointer.empty() || pointer == "/") {
        return make_error_code(JsonErrc::Success);
    }
    
    if (pointer[0] != '/') {
        errMsg = "JSON pointer must start with '/'";
        return make_error_code(JsonErrc::ParseError);
    }
    
    size_t start = 1;
    for (size_t i = 1; i <= pointer.length(); ++i) {
        if (i == pointer.length() || pointer[i] == '/') {
            std::string unescapedToken;
            auto ec = unescapeJsonPointerSafe(pointer.substr(start, i - start), unescapedToken, errMsg);
            if (ec) {
                return ec;
            }
            tokens.push_back(std::move(unescapedToken));
            start = i + 1;
        }
    }
    
    return make_error_code(JsonErrc::Success);
}

std::string JsonValue::unescapeJsonPointer(std::string_view token) {
    std::string result;
    for (size_t i = 0; i < token.length(); ++i) {
        if (token[i] == '~') {
            if (i + 1 < token.length()) {
                switch (token[i + 1]) {
                    case '1':
                        result += '/';
                        break;
                    case '0':
                        result += '~';
                        break;
                    default:
                        throw std::runtime_error("Invalid JSON pointer escape sequence: ~" + std::string(1, token[i + 1]));
                }
                ++i; // Skip the next character as it is part of the escape sequence
            } else {
                throw std::runtime_error("Incomplete escape sequence at end of token");
            }
        } else {
            result += token[i];
        }
    }
    return result;
}

std::error_code JsonValue::unescapeJsonPointerSafe(std::string_view token, std::string& result, std::string& errMsg) {
    result.clear();
    for (size_t i = 0; i < token.length(); ++i) {
        if (token[i] == '~') {
            if (i + 1 < token.length()) {
                switch (token[i + 1]) {
                    case '1':
                        result += '/';
                        break;
                    case '0':
                        result += '~';
                        break;
                    default:
                        errMsg = "Invalid JSON pointer escape sequence: ~" + std::string(1, token[i + 1]);
                        return make_error_code(JsonErrc::ParseError);
                }
                ++i; // Skip the next character as it is part of the escape sequence
            } else {
                errMsg = "Incomplete escape sequence at end of token";
                return make_error_code(JsonErrc::ParseError);
            }
        } else {
            result += token[i];
        }
    }
    return make_error_code(JsonErrc::Success);
}

JsonValue& JsonValue::at(std::string_view jsonPointer) {
    auto tokens = parseJsonPointer(jsonPointer);
    JsonValue* current = this;
    
    for (const auto& token : tokens) {
        if (current->isArray()) {
            try {
                // Array index
                size_t index = std::stoull(token);
                if (index >= current->size()) {
                    throw std::runtime_error("Array index out of bounds: " + token);
                }
                current = &((*current)[index]);
            } catch (const std::exception& e) {
                throw std::runtime_error("Invalid array index: " + token);
            }
        } else if (current->isObject()) {
            auto jsonStr = current->toJson(0);
            // Object key
            std::string adjustedToken = token;
            if (!current->contains(adjustedToken)) {
                throw std::runtime_error("Property not found: " + adjustedToken);
            }

            current = &((*current)[adjustedToken]);
        } else {
            throw std::runtime_error("Cannot index into non-container type");
        }
    }
    
    return *current;
}

const JsonValue& JsonValue::at(std::string_view jsonPointer) const {
    return const_cast<JsonValue*>(this)->at(jsonPointer);
}

std::error_code JsonValue::atSafe(std::string_view jsonPointer, JsonValue*& result, std::string& errMsg) {
    std::vector<std::string> tokens;
    auto ec = parseJsonPointerSafe(jsonPointer, tokens, errMsg);
    if (ec) {
        return ec;
    }
    
    JsonValue* current = this;
    
    for (const auto& token : tokens) {
        if (current->isArray()) {
            // Array index
            size_t index;
            auto [ptr, parseEc] = std::from_chars(token.data(), token.data() + token.size(), index);
            if (parseEc != std::errc{}) {
                errMsg = "Invalid array index: " + token;
                return make_error_code(JsonErrc::ParseError);
            }
            if (index >= current->size()) {
                errMsg = "Array index out of bounds: " + token;
                return make_error_code(JsonErrc::OutOfRange);
            }
            current = &((*current)[index]);
        } else if (current->isObject()) {
            // Object key
            std::string adjustedToken = token;
            if (!current->contains(adjustedToken)) {
                errMsg = "Property not found: " + adjustedToken;
                return make_error_code(JsonErrc::OutOfRange);
            }
            current = &((*current)[adjustedToken]);
        } else {
            errMsg = "Cannot index into non-container type";
            return make_error_code(JsonErrc::TypeError);
        }
    }
    
    result = current;
    return make_error_code(JsonErrc::Success);
}

std::error_code JsonValue::atSafe(std::string_view jsonPointer, const JsonValue*& result, std::string& errMsg) const {
    JsonValue* nonConstResult = nullptr;
    auto ec = const_cast<JsonValue*>(this)->atSafe(jsonPointer, nonConstResult, errMsg);
    if (!ec) {
        result = nonConstResult;
    }
    return ec;
}

// Literal operator
namespace literals {
    JsonValue operator""_json(const char* str, size_t len) {
        return JsonValue::parse(std::string_view(str, len));
    }
}

// Simplified JSONPath implementation - delegates to JsonFilter for better separation of concerns
// These methods are provided for backward compatibility
bool JsonValue::pathExists(const std::string& jsonpath_expression) const {
    static JsonFilter defaultFilter = JsonFilter::createDefault();
    return defaultFilter.pathExists(*this, jsonpath_expression);
}

const JsonValue* JsonValue::selectFirst(const std::string& jsonpath_expression) const {
    static JsonFilter defaultFilter = JsonFilter::createDefault();
    return defaultFilter.selectFirst(*this, jsonpath_expression);
}

std::vector<const JsonValue*> JsonValue::selectAll(const std::string& jsonpath_expression) const {
    static JsonFilter defaultFilter = JsonFilter::createDefault();
    return defaultFilter.selectAll(*this, jsonpath_expression);
}

std::vector<JsonValue> JsonValue::selectValues(const std::string& jsonpath_expression) const {
    static JsonFilter defaultFilter = JsonFilter::createDefault();
    return defaultFilter.selectValues(*this, jsonpath_expression);
}

// === Streaming Query Convenience Methods ===
// These provide simple access to JsonStreamingQuery functionality

std::optional<std::pair<const JsonValue*, std::string>> JsonValue::findFirst(const std::string& expression) const {
    // Use JsonFilter for basic implementation
    static JsonFilter defaultFilter = JsonFilter::createDefault();
    auto result = defaultFilter.selectFirst(*this, expression);
    if (result) {
        return std::make_pair(result, "$[0]"); // Simplified path
    }
    return std::nullopt;
}

size_t JsonValue::countMatches(const std::string& expression, size_t maxCount) const {
    // Use JsonFilter for basic implementation
    static JsonFilter defaultFilter = JsonFilter::createDefault();
    auto results = defaultFilter.selectAll(*this, expression);
    
    if (maxCount > 0 && results.size() > maxCount) {
        return maxCount;
    }
    return results.size();
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

// Safe parsing and serialization: catch exceptions and return error codes
namespace JsonStruct {

std::error_code JsonValue::toJson(std::string& out,
                                       std::string& errMsg,
                                       const SerializeOptions& options) const {
    try {
        out = this->dump(options);
        errMsg.clear();
        return make_error_code(JsonErrc::Success);
    } catch (const std::exception& ex) {
        errMsg = ex.what();
        return make_error_code(JsonErrc::UnknownError);
    }
}

} // namespace JsonStruct
