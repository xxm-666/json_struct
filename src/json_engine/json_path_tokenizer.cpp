#include "json_path_tokenizer.h"
#include <cctype>

namespace jsonpath {

std::vector<Token> JsonPathTokenizer::tokenize(std::string_view expr) {
    std::vector<Token> tokens;
    size_t i = 0;
    int left_bracket_count = 0;

    while (i < expr.length()) {
        char c = expr[i];
        
        switch (c) {
            case '$':
                tokens.emplace_back(TokenType::ROOT, "$", i);
                ++i;
                break;
                
            case '.':
                if (i + 1 < expr.length() && expr[i + 1] == '.') {
                    tokens.emplace_back(TokenType::RECURSIVE, "..", i);
                    i += 2;
                } else {
                    tokens.emplace_back(TokenType::DOT, ".", i);
                    ++i;
                }
                break;
                
            case '[':
                left_bracket_count++;
                tokens.emplace_back(TokenType::BRACKET_OPEN, "[", i);
                ++i;
                break;
                
            case ']':
                left_bracket_count--;
                tokens.emplace_back(TokenType::BRACKET_CLOSE, "]", i);
                ++i;
                break;
                
            case '*':
                tokens.emplace_back(TokenType::WILDCARD, "*", i);
                ++i;
                break;
                
            case ':':
                tokens.emplace_back(TokenType::SLICE, ":", i);
                ++i;
                break;
                
            case '?':
                tokens.emplace_back(TokenType::FILTER, "?", i);
                ++i;
                break;
                
            case ',':
                if (left_bracket_count == 0) {
                    tokens.emplace_back(TokenType::COMMA, ",", i);
                } else {
                    // Inside brackets, comma is part of union
                    tokens.emplace_back(TokenType::COMMA, ",", i);
                }
                ++i;
                break;
                
            case '\'':
            case '"':
                try {
                    std::string str_value = parseString(expr, i, c);
                    tokens.emplace_back(TokenType::STRING, str_value, i - str_value.length() - 2);
                } catch (const std::exception&) {
                    throw JsonPathTokenizerException("Unterminated string", i);
                }
                break;
                
            case '(':
                tokens.emplace_back(TokenType::PAREN_OPEN, "(", i);
                ++i;
                break;

            case ')':
                tokens.emplace_back(TokenType::PAREN_CLOSE, ")", i);
                ++i;
                break;

            case '@':
                tokens.emplace_back(TokenType::AT, "@", i);
                ++i;
                break;

            case '<':
            case '>':
            case '=':
            case '!': {
                std::string op_str;
                op_str += c;
                size_t start_pos = i++;
                
                if (i < expr.length()) {
                    if (c == '=' && expr[i] == '~') {
                        op_str += expr[i++];
                        tokens.emplace_back(TokenType::REGEX, op_str, start_pos);
                    } else if ((c == '!' || c == '=' || c == '<' || c == '>') && expr[i] == '=') {
                        op_str += expr[i++];
                        if (c == '!') {
                            tokens.emplace_back(TokenType::NOT_EQUAL, op_str, start_pos);
                        } else {
                            tokens.emplace_back(TokenType::EQUAL, op_str, start_pos);
                        }
                    } else if (c == '=') {
                        tokens.emplace_back(TokenType::EQUAL, op_str, start_pos);
                    } else {
                        tokens.emplace_back(c == '<' ? TokenType::LESS : TokenType::GREATER, op_str, start_pos);
                    }
                } else {
                    if (c == '=') {
                        tokens.emplace_back(TokenType::EQUAL, op_str, start_pos);
                    } else {
                        tokens.emplace_back(c == '<' ? TokenType::LESS : TokenType::GREATER, op_str, start_pos);
                    }
                }
                break;
            }
            
            case '&': 
            case '|': {
                if (i + 1 < expr.length() && expr[i + 1] == c) {
                    std::string op_str;
                    op_str += c;
                    op_str += c;
                    tokens.emplace_back(c == '&' ? TokenType::AND : TokenType::OR, op_str, i);
                    i += 2;
                } else {
                    throw JsonPathTokenizerException("Invalid operator", i);
                }
                break;
            }
            
            case '/': {
                // Handle regex patterns like /pattern/flags
                if (i + 1 < expr.length()) {
                    size_t start = i;
                    std::string regex_pattern = "/";
                    ++i; // Skip first /
                    
                    while (i < expr.length() && expr[i] != '/') {
                        if (expr[i] == '\\' && i + 1 < expr.length()) {
                            regex_pattern += expr[i++]; // Include backslash
                        }
                        regex_pattern += expr[i++];
                    }
                    
                    if (i < expr.length() && expr[i] == '/') {
                        regex_pattern += '/';
                        ++i;
                        
                        // Parse flags (like 'i' for case-insensitive)
                        while (i < expr.length() && std::isalpha(expr[i])) {
                            regex_pattern += expr[i++];
                        }
                        
                        tokens.emplace_back(TokenType::STRING, regex_pattern, start);
                    } else {
                        throw JsonPathTokenizerException("Unterminated regex", start);
                    }
                } else {
                    throw JsonPathTokenizerException("Unexpected character", i);
                }
                break;
            }
            
            case 'i': {
                // Check for 'in' operator
                if (i + 1 < expr.length() && expr[i + 1] == 'n' && 
                    (i + 2 >= expr.length() || !isValidIdentifier(expr[i + 2]))) {
                    tokens.emplace_back(TokenType::IN, "in", i);
                    i += 2;
                } else {
                    // Regular identifier
                    std::string identifier = parseIdentifier(expr, i);
                    tokens.emplace_back(TokenType::IDENTIFIER, identifier, i - identifier.length());
                }
                break;
            }
            
            default:
                if (std::isspace(c)) {
                    ++i; // Skip whitespace
                } else if (std::isdigit(c) || c == '-') {
                    std::string number = parseNumber(expr, i);
                    tokens.emplace_back(TokenType::NUMBER, number, i - number.length());
                } else if (isValidIdentifier(c)) {
                    std::string identifier = parseIdentifier(expr, i);
                    tokens.emplace_back(TokenType::IDENTIFIER, identifier, i - identifier.length());
                } else {
                    throw JsonPathTokenizerException("Unexpected character", i);
                }
                break;
        }
    }
    
    tokens.emplace_back(TokenType::END, "", expr.length());
    return tokens;
}

bool JsonPathTokenizer::isValidIdentifier(char c) {
    return std::isalnum(c) || c == '_' || c == '$';
}

std::string JsonPathTokenizer::parseString(std::string_view expr, size_t& pos, char quote_char) {
    std::string result;
    ++pos; // Skip opening quote
    
    while (pos < expr.length()) {
        char c = expr[pos];
        
        if (c == quote_char) {
            ++pos; // Skip closing quote
            return result;
        } else if (c == '\\' && pos + 1 < expr.length()) {
            ++pos;
            char escaped = expr[pos];
            switch (escaped) {
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case 'r': result += '\r'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case '"': result += '"'; break;
                case '\'': result += '\''; break;
                default: 
                    result += '\\';
                    result += escaped;
                    break;
            }
            ++pos;
        } else {
            result += c;
            ++pos;
        }
    }
    
    throw JsonPathTokenizerException("Unterminated string", pos);
}

std::string JsonPathTokenizer::parseNumber(std::string_view expr, size_t& pos) {
    std::string result;
    
    if (pos < expr.length() && expr[pos] == '-') {
        result += expr[pos++];
    }
    
    while (pos < expr.length() && std::isdigit(expr[pos])) {
        result += expr[pos++];
    }
    
    if (pos < expr.length() && expr[pos] == '.') {
        result += expr[pos++];
        while (pos < expr.length() && std::isdigit(expr[pos])) {
            result += expr[pos++];
        }
    }
    
    return result;
}

std::string JsonPathTokenizer::parseIdentifier(std::string_view expr, size_t& pos) {
    std::string result;
    
    while (pos < expr.length() && isValidIdentifier(expr[pos])) {
        result += expr[pos++];
    }
    
    return result;
}

} // namespace jsonpath
