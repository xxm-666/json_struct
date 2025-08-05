#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>

namespace jsonpath {

/**
 * @brief Token types for JSONPath expression parsing
 */
enum class TokenType {
    ROOT,           // $
    DOT,            // .
    BRACKET_OPEN,   // [
    BRACKET_CLOSE,  // ]
    IDENTIFIER,     // property name
    STRING,         // quoted string
    NUMBER,         // numeric index
    WILDCARD,       // *
    RECURSIVE,      // ..
    SLICE,          // :
    FILTER,         // ?
    COMMA,          // ,
    PAREN_OPEN,     // (
    PAREN_CLOSE,    // )
    AT,             // @
    LESS,           // <
    GREATER,        // >
    EQUAL,          // =
    NOT_EQUAL,      // !=
    AND,            // &&
    OR,             // ||
    REGEX,          // =~
    IN,             // in
    END             // end of input
};

/**
 * @brief Token structure for JSONPath parsing
 */
struct Token {
    TokenType type;
    std::string value;
    size_t position;
    
    Token(TokenType t, std::string v = "", size_t pos = 0) 
        : type(t), value(std::move(v)), position(pos) {}
};

/**
 * @brief JSONPath tokenizer - responsible for breaking down expressions into tokens
 */
class JsonPathTokenizer {
public:
    /**
     * @brief Tokenize a JSONPath expression
     * @param expr The expression to tokenize
     * @return Vector of tokens
     */
    static std::vector<Token> tokenize(std::string_view expr);

private:
    static bool isValidIdentifier(char c);
    static std::string parseString(std::string_view expr, size_t& pos, char quote_char);
    static std::string parseNumber(std::string_view expr, size_t& pos);
    static std::string parseIdentifier(std::string_view expr, size_t& pos);
};

/**
 * @brief JSONPath tokenizer exception
 */
class JsonPathTokenizerException : public std::runtime_error {
public:
    JsonPathTokenizerException(const std::string& message, size_t position)
        : std::runtime_error(message + " at position " + std::to_string(position))
        , position_(position) {}
    
    size_t position() const { return position_; }

private:
    size_t position_;
};

} // namespace jsonpath
