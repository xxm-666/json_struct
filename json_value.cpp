#include "json_value.h"
#include <cctype>

namespace JsonStruct {

void JsonValue::dumpImpl(std::ostream& os, int indent, int currentIndent) const {
    switch (type()) {
        case Null:
            os << "null";
            break;
        case Bool:
            os << (toBool() ? "true" : "false");
            break;
        case Number:
            os << toDouble();
            break;
        case String:
            os << "\"" << escapeString(toString()) << "\"";
            break;
        case Array: {
            os << "[";
            const auto& arr = toArray();
            for (size_t i = 0; i < arr.size(); ++i) {
                if (i > 0) os << ",";
                if (indent >= 0) {
                    os << "\n" << std::string(currentIndent + indent, ' ');
                }
                arr[i].dumpImpl(os, indent, currentIndent + indent);
            }
            if (indent >= 0 && !arr.empty()) {
                os << "\n" << std::string(currentIndent, ' ');
            }
            os << "]";
            break;
        }
        case Object: {
            os << "{";
            const auto& obj = toObject();
            bool first = true;
            for (ObjectType::const_iterator it = obj.begin(); it != obj.end(); ++it) {
                if (!first) os << ",";
                first = false;
                if (indent >= 0) {
                    os << "\n" << std::string(currentIndent + indent, ' ');
                }
                os << "\"" << escapeString(it->first) << "\":";
                if (indent >= 0) os << " ";
                it->second.dumpImpl(os, indent, currentIndent + indent);
            }
            if (indent >= 0 && !obj.empty()) {
                os << "\n" << std::string(currentIndent, ' ');
            }
            os << "}";
            break;
        }
    }
}

std::string JsonValue::escapeString(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    return result;
}

JsonValue JsonValue::parseValue(const std::string& str, size_t& pos) {
    skipWhitespace(str, pos);
    if (pos >= str.length()) {
        throw std::runtime_error("Unexpected end of input");
    }

    char c = str[pos];
    if (c == 'n') {
        return parseNull(str, pos);
    } else if (c == 't' || c == 'f') {
        return parseBool(str, pos);
    } else if (c == '"') {
        return parseString(str, pos);
    } else if (c == '[') {
        return parseArray(str, pos);
    } else if (c == '{') {
        return parseObject(str, pos);
    } else if (c == '-' || std::isdigit(c)) {
        return parseNumber(str, pos);
    } else {
        throw std::runtime_error("Unexpected character: " + std::string(1, c));
    }
}

void JsonValue::skipWhitespace(const std::string& str, size_t& pos) {
    while (pos < str.length() && std::isspace(str[pos])) {
        ++pos;
    }
}

JsonValue JsonValue::parseNull(const std::string& str, size_t& pos) {
    if (str.substr(pos, 4) == "null") {
        pos += 4;
        return JsonValue();
    }
    throw std::runtime_error("Invalid null value");
}

JsonValue JsonValue::parseBool(const std::string& str, size_t& pos) {
    if (str.substr(pos, 4) == "true") {
        pos += 4;
        return JsonValue(true);
    } else if (str.substr(pos, 5) == "false") {
        pos += 5;
        return JsonValue(false);
    }
    throw std::runtime_error("Invalid boolean value");
}

JsonValue JsonValue::parseNumber(const std::string& str, size_t& pos) {
    size_t start = pos;
    if (str[pos] == '-') ++pos;
    while (pos < str.length() && std::isdigit(str[pos])) ++pos;
    if (pos < str.length() && str[pos] == '.') {
        ++pos;
        while (pos < str.length() && std::isdigit(str[pos])) ++pos;
    }
    if (pos < str.length() && (str[pos] == 'e' || str[pos] == 'E')) {
        ++pos;
        if (pos < str.length() && (str[pos] == '+' || str[pos] == '-')) ++pos;
        while (pos < str.length() && std::isdigit(str[pos])) ++pos;
    }
    return JsonValue(std::stod(str.substr(start, pos - start)));
}

JsonValue JsonValue::parseString(const std::string& str, size_t& pos) {
    if (str[pos] != '"') {
        throw std::runtime_error("Expected '\"'");
    }
    ++pos;
    std::string result;
    while (pos < str.length() && str[pos] != '"') {
        if (str[pos] == '\\') {
            ++pos;
            if (pos >= str.length()) {
                throw std::runtime_error("Unexpected end of string");
            }
            switch (str[pos]) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                default: result += str[pos]; break;
            }
        } else {
            result += str[pos];
        }
        ++pos;
    }
    if (pos >= str.length()) {
        throw std::runtime_error("Unterminated string");
    }
    ++pos; // skip closing quote
    return JsonValue(result);
}

JsonValue JsonValue::parseArray(const std::string& str, size_t& pos) {
    if (str[pos] != '[') {
        throw std::runtime_error("Expected '['");
    }
    ++pos;
    
    JsonValue::ArrayType arr;
    skipWhitespace(str, pos);
    
    if (pos < str.length() && str[pos] == ']') {
        ++pos;
        return JsonValue(arr);
    }
    
    while (true) {
        arr.push_back(parseValue(str, pos));
        skipWhitespace(str, pos);
        
        if (pos >= str.length()) {
            throw std::runtime_error("Unterminated array");
        }
        
        if (str[pos] == ']') {
            ++pos;
            break;
        } else if (str[pos] == ',') {
            ++pos;
            skipWhitespace(str, pos);
        } else {
            throw std::runtime_error("Expected ',' or ']'");
        }
    }
    
    return JsonValue(arr);
}

JsonValue JsonValue::parseObject(const std::string& str, size_t& pos) {
    if (str[pos] != '{') {
        throw std::runtime_error("Expected '{'");
    }
    ++pos;
    
    JsonValue::ObjectType obj;
    skipWhitespace(str, pos);
    
    if (pos < str.length() && str[pos] == '}') {
        ++pos;
        return JsonValue(obj);
    }
    
    while (true) {
        skipWhitespace(str, pos);
        JsonValue key = parseString(str, pos);
        skipWhitespace(str, pos);
        
        if (pos >= str.length() || str[pos] != ':') {
            throw std::runtime_error("Expected ':'");
        }
        ++pos;
        
        JsonValue value = parseValue(str, pos);
        obj[key.toString()] = value;
        
        skipWhitespace(str, pos);
        
        if (pos >= str.length()) {
            throw std::runtime_error("Unterminated object");
        }
        
        if (str[pos] == '}') {
            ++pos;
            break;
        } else if (str[pos] == ',') {
            ++pos;
        } else {
            throw std::runtime_error("Expected ',' or '}'");
        }
    }
    
    return JsonValue(obj);
}

} // namespace JsonStruct
