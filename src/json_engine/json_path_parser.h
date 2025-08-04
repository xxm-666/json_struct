#pragma once

#include "json_path_tokenizer.h"
#include <string>
#include <vector>
#include <optional>
#include <climits>

namespace jsonpath {

/**
 * @brief JSONPath expression node types
 */
enum class NodeType {
    ROOT,           // $
    PROPERTY,       // .name or ['name']
    INDEX,          // [0] or [-1]
    SLICE,          // [1:3] or [1:3:2] (with step)
    WILDCARD,       // *
    RECURSIVE,      // ..
    FILTER,         // [?(...)]
    UNION           // Multiple paths separated by comma
};

/**
 * @brief JSONPath expression node
 */
struct PathNode {
    NodeType type;
    std::string property;           // For PROPERTY nodes
    int index = 0;                  // For INDEX nodes  
    int slice_start = 0;            // For SLICE nodes
    int slice_end = INT_MAX;        // For SLICE nodes (INT_MAX = end of array)
    int slice_step = 1;             // For SLICE nodes with step
    std::string filter_expr;        // For FILTER nodes
    std::vector<int> union_indices; // For UNION index nodes like [0,2,4]
    std::vector<std::string> union_paths; // For UNION path expressions
    
    PathNode(NodeType t) : type(t) {}
    PathNode(NodeType t, std::string prop) : type(t), property(std::move(prop)) {}
    PathNode(NodeType t, int idx) : type(t), index(idx) {}
};

/**
 * @brief JSONPath parser - converts tokens into AST (Abstract Syntax Tree)
 */
class JsonPathParser {
public:
    /**
     * @brief Parse tokens into path nodes
     * @param tokens Vector of tokens to parse
     * @return Vector of path nodes representing the AST
     */
    static std::vector<PathNode> parse(const std::vector<Token>& tokens);
    
    /**
     * @brief Parse a union expression (comma-separated paths)
     * @param expr The expression string containing commas
     * @return Vector of path nodes with union node
     */
    static std::vector<PathNode> parseUnionExpression(const std::string& expr);
    
    /**
     * @brief Check if expression has top-level commas (not inside brackets)
     * @param expr Expression to check
     * @return True if has top-level commas
     */
    static bool hasTopLevelComma(const std::string& expr);

private:
private:
    /**
     * @brief Parse a single node from tokens
     * @param tokens Vector of tokens
     * @param pos Current position in tokens (will be updated)
     * @return Parsed path node
     */
    static PathNode parseNode(const std::vector<Token>& tokens, size_t& pos);
    
    /**
     * @brief Parse filter expression inside brackets
     * @param tokens Vector of tokens
     * @param pos Current position (should be after '?')
     * @return Filter expression string
     */
    static std::string parseFilterExpression(const std::vector<Token>& tokens, size_t& pos);
    
    /**
     * @brief Parse slice notation like [1:3:2]
     * @param tokens Vector of tokens
     * @param pos Current position
     * @param start Output for slice start
     * @param end Output for slice end
     * @param step Output for slice step
     */
    static void parseSlice(const std::vector<Token>& tokens, size_t& pos, 
                          int& start, int& end, int& step);
};

/**
 * @brief JSONPath parser exception
 */
class JsonPathParserException : public std::runtime_error {
public:
    JsonPathParserException(const std::string& message, size_t position)
        : std::runtime_error(message + " at position " + std::to_string(position))
        , position_(position) {}
    
    size_t position() const { return position_; }

private:
    size_t position_;
};

} // namespace jsonpath
