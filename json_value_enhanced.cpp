#include "json_value_enhanced.h"
#include <cctype>
#include <algorithm>
#include <cstdio>
#include <iomanip>
#include <sstream>

namespace JsonStruct {

void JsonValueEnhanced::dumpImpl(std::ostream& os, const SerializeOptions& options, int currentIndent) const {
    visit([&](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        
        if constexpr (std::is_same_v<T, std::monostate>) {
            os << "null";
        } else if constexpr (std::is_same_v<T, bool>) {
            os << (value ? "true" : "false");
        } else if constexpr (std::is_same_v<T, JsonNumber>) {
            // 使用JsonNumber的智能格式化
            if (value.isInteger()) {
                os << value.toInteger();
            } else {
                // 浮点数使用指定精度
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
            
            // 收集键并可选排序
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

std::string JsonValueEnhanced::escapeString(std::string_view str, bool escapeUnicode) {
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
                    // 控制字符转为Unicode转义
                    result += "\\u";
                    char hex[5];
                    std::sprintf(hex, "%04x", c);
                    result += hex;
                } else if (escapeUnicode && c >= 0x80) {
                    // 可选的Unicode字符转义
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

JsonValueEnhanced JsonValueEnhanced::parseValue(ParseContext& ctx) {
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
        case '[': return parseArray(ctx);
        case '{': return parseObject(ctx);
        case '-': case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return parseNumber(ctx);
        default:
            throw std::runtime_error("Unexpected character '" + std::string(1, c) + 
                                    "' at " + ctx.locationInfo());
    }
}

void JsonValueEnhanced::skipWhitespace(ParseContext& ctx) {
    while (ctx.hasMore()) {
        char c = ctx.peek();
        if (std::isspace(c)) {
            ctx.advance(c);
        } else if (ctx.options.allowComments) {
            if (c == '/' && ctx.peek(1) == '/') {
                // 单行注释
                while (ctx.hasMore() && ctx.peek() != '\n') {
                    ctx.advance(ctx.peek());
                }
            } else if (c == '/' && ctx.peek(1) == '*') {
                // 多行注释
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

JsonValueEnhanced JsonValueEnhanced::parseNull(ParseContext& ctx) {
    constexpr std::string_view nullStr = "null";
    
    if (ctx.source.substr(ctx.position, nullStr.length()) == nullStr) {
        for (size_t i = 0; i < nullStr.length(); ++i) {
            ctx.advance(ctx.peek());
        }
        return JsonValueEnhanced{};
    }
    throw std::runtime_error("Invalid null value at " + ctx.locationInfo());
}

JsonValueEnhanced JsonValueEnhanced::parseBool(ParseContext& ctx) {
    constexpr std::string_view trueStr = "true";
    constexpr std::string_view falseStr = "false";
    
    if (ctx.source.substr(ctx.position, trueStr.length()) == trueStr) {
        for (size_t i = 0; i < trueStr.length(); ++i) {
            ctx.advance(ctx.peek());
        }
        return JsonValueEnhanced(true);
    } else if (ctx.source.substr(ctx.position, falseStr.length()) == falseStr) {
        for (size_t i = 0; i < falseStr.length(); ++i) {
            ctx.advance(ctx.peek());
        }
        return JsonValueEnhanced(false);
    }
    throw std::runtime_error("Invalid boolean value at " + ctx.locationInfo());
}

JsonValueEnhanced JsonValueEnhanced::parseNumber(ParseContext& ctx) {
    size_t start = ctx.position;
    bool hasDecimal = false;
    bool hasExponent = false;
    
    // 符号
    if (ctx.peek() == '-') {
        ctx.advance(ctx.peek());
    }
    
    if (!ctx.hasMore() || !std::isdigit(ctx.peek())) {
        throw std::runtime_error("Invalid number format at " + ctx.locationInfo());
    }
    
    // 整数部分
    if (ctx.peek() == '0') {
        ctx.advance(ctx.peek());
        // JSON标准：0后面不能直接跟数字
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
    
    // 小数部分
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
    
    // 指数部分
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
    
    // 根据格式智能选择解析方式
    if (hasDecimal || hasExponent) {
        // 浮点数
        double result;
        auto [ptr, ec] = std::from_chars(numStr.data(), numStr.data() + numStr.size(), result);
        if (ec != std::errc{}) {
            throw std::runtime_error("Failed to parse number '" + std::string(numStr) + 
                                    "' at " + ctx.locationInfo());
        }
        return JsonValueEnhanced(result);
    } else {
        // 整数：先尝试解析为int64_t
        try {
            int64_t intResult;
            auto [ptr, ec] = std::from_chars(numStr.data(), numStr.data() + numStr.size(), intResult);
            if (ec == std::errc{}) {
                return JsonValueEnhanced(static_cast<long long>(intResult));
            }
        } catch (...) {
            // 整数解析失败，回退到double
        }
        
        // 回退到double解析
        double result;
        auto [ptr, ec] = std::from_chars(numStr.data(), numStr.data() + numStr.size(), result);
        if (ec != std::errc{}) {
            throw std::runtime_error("Failed to parse number '" + std::string(numStr) + 
                                    "' at " + ctx.locationInfo());
        }
        return JsonValueEnhanced(result);
    }
}

std::string JsonValueEnhanced::parseUnicodeEscape(std::string_view str, size_t& pos) {
    if (pos + 5 >= str.length()) {
        throw std::runtime_error("Invalid Unicode escape sequence: too short");
    }
    
    std::string hexStr(str.substr(pos + 2, 4));
    pos += 6; // 跳过 \uXXXX
    
    // 解析十六进制
    unsigned int codepoint;
    auto [ptr, ec] = std::from_chars(hexStr.data(), hexStr.data() + 4, codepoint, 16);
    if (ec != std::errc{}) {
        throw std::runtime_error("Invalid Unicode escape sequence: '" + hexStr + "'");
    }
    
    // 处理代理对 (surrogate pairs) for UTF-16
    if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
        // 高代理，需要后续的低代理
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
        
        // 合并代理对
        codepoint = 0x10000 + ((codepoint & 0x3FF) << 10) + (lowSurrogate & 0x3FF);
    }
    
    // 转换为UTF-8
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

JsonValueEnhanced JsonValueEnhanced::parseString(ParseContext& ctx) {
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
                    // Unicode转义支持
                    size_t tempPos = ctx.position - 1; // 回到'\'位置
                    try {
                        result += parseUnicodeEscape(ctx.source, tempPos);
                        ctx.position = tempPos;
                        continue; // 跳过下面的advance
                    } catch (const std::exception&) {
                        throw std::runtime_error("Invalid Unicode escape at " + ctx.locationInfo());
                    }
                }
                default:
                    if (ctx.options.strictMode) {
                        throw std::runtime_error("Invalid escape sequence '\\" + 
                                               std::string(1, escape) + "' at " + ctx.locationInfo());
                    } else {
                        // 非严格模式：保留未知转义序列
                        result += '\\';
                        result += escape;
                    }
                    break;
            }
            ctx.advance(ctx.peek());
        } else {
            char c = ctx.peek();
            
            // 验证UTF-8（如果启用）
            if (ctx.options.validateUtf8 && static_cast<unsigned char>(c) >= 0x80) {
                // 简单的UTF-8验证
                // 这里可以实现更完整的UTF-8验证
            }
            
            result += c;
            ctx.advance(c);
        }
    }
    
    if (!ctx.hasMore()) {
        throw std::runtime_error("Unterminated string at " + ctx.locationInfo());
    }
    
    ctx.advance(ctx.peek()); // skip closing quote
    return JsonValueEnhanced(std::move(result));
}

JsonValueEnhanced JsonValueEnhanced::parseArray(ParseContext& ctx) {
    if (ctx.peek() != '[') {
        throw std::runtime_error("Expected '[' at " + ctx.locationInfo());
    }
    ctx.advance(ctx.peek()); // skip '['
    ++ctx.depth;
    
    ArrayType arr;
    skipWhitespace(ctx);
    
    // 空数组
    if (ctx.hasMore() && ctx.peek() == ']') {
        ctx.advance(ctx.peek());
        --ctx.depth;
        return JsonValueEnhanced(std::move(arr));
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
            
            // 处理尾随逗号
            if (ctx.options.allowTrailingCommas && ctx.hasMore() && ctx.peek() == ']') {
                ctx.advance(ctx.peek());
                break;
            }
        } else {
            throw std::runtime_error("Expected ',' or ']' at " + ctx.locationInfo());
        }
    }
    
    --ctx.depth;
    return JsonValueEnhanced(std::move(arr));
}

JsonValueEnhanced JsonValueEnhanced::parseObject(ParseContext& ctx) {
    if (ctx.peek() != '{') {
        throw std::runtime_error("Expected '{' at " + ctx.locationInfo());
    }
    ctx.advance(ctx.peek()); // skip '{'
    ++ctx.depth;
    
    ObjectType obj;
    skipWhitespace(ctx);
    
    // 空对象
    if (ctx.hasMore() && ctx.peek() == '}') {
        ctx.advance(ctx.peek());
        --ctx.depth;
        return JsonValueEnhanced(std::move(obj));
    }
    
    while (true) {
        skipWhitespace(ctx);
        
        // 解析键
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
        
        // 解析值
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
            
            // 处理尾随逗号
            if (ctx.options.allowTrailingCommas && ctx.hasMore() && ctx.peek() == '}') {
                ctx.advance(ctx.peek());
                break;
            }
        } else {
            throw std::runtime_error("Expected ',' or '}' at " + ctx.locationInfo());
        }
    }
    
    --ctx.depth;
    return JsonValueEnhanced(std::move(obj));
}

bool JsonValueEnhanced::isValidUtf8(std::string_view str) {
    // 简化的UTF-8验证
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

// JSON指针实现 (RFC 6901)
std::vector<std::string> JsonValueEnhanced::parseJsonPointer(std::string_view pointer) {
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

std::string JsonValueEnhanced::unescapeJsonPointer(std::string_view token) {
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

JsonValueEnhanced& JsonValueEnhanced::at(std::string_view jsonPointer) {
    auto tokens = parseJsonPointer(jsonPointer);
    JsonValueEnhanced* current = this;
    
    for (const auto& token : tokens) {
        if (current->isArray()) {
            // 数组索引
            try {
                size_t index = std::stoull(token);
                current = &((*current)[index]);
            } catch (const std::exception&) {
                throw std::runtime_error("Invalid array index: " + token);
            }
        } else if (current->isObject()) {
            // 对象键
            current = &((*current)[token]);
        } else {
            throw std::runtime_error("Cannot index into non-container type");
        }
    }
    
    return *current;
}

const JsonValueEnhanced& JsonValueEnhanced::at(std::string_view jsonPointer) const {
    return const_cast<JsonValueEnhanced*>(this)->at(jsonPointer);
}

// 字面量操作符
namespace literals {
    JsonValueEnhanced operator""_json(const char* str, size_t len) {
        return JsonValueEnhanced::parse(std::string_view(str, len));
    }
}

// 流操作符
std::ostream& operator<<(std::ostream& os, const JsonValueEnhanced& value) {
    os << value.dump();
    return os;
}

std::istream& operator>>(std::istream& is, JsonValueEnhanced& value) {
    std::string json((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
    value = JsonValueEnhanced::parse(json);
    return is;
}

} // namespace JsonStruct
