#pragma once

#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <stdexcept>

// Forward declaration
namespace JsonStruct {
    class JsonValue;
}

/**
 * @brief JSONPath query language implementation
 * 
 * This is the main entry point for JSONPath evaluation. It uses a modular architecture:
 * - JsonPathTokenizer: Breaks expressions into tokens
 * - JsonPathParser: Converts tokens into AST nodes
 * - SimplePathEvaluator: Handles basic path traversal
 * - FilterEvaluator: Handles filter expressions [?(...)]
 * - AdvancedEvaluator: Handles recursive descent (..) and union operations
 * 
 * Supports JSONPath expressions:
 * - Root: $ (refers to the entire JSON document)
 * - Child access: $.store or $['store']
 * - Array index: $.array[0] or $.array[-1] (negative indexing)
 * - Array slice: $.array[1:3] or $.array[:2] or $.array[1:]
 * - Wildcard: $.*.name or $[*]
 * - Recursive descent: $..name (finds all 'name' properties at any level)
 * - Filter expressions: $.array[?(@.price < 10)]
 * - Multiple selections: $.store,$.warehouse
 * 
 * Examples:
 * - "$.store.book[0].title" - Get title of first book
 * - "$.store.book[*].author" - Get all book authors  
 * - "$..price" - Get all price values recursively
 * - "$.store.book[?(@.price < 10)]" - Filter books by price
 * - "$.store.book[-1:]" - Get last book(s)
 */

namespace jsonpath {

// Re-export types from modules for backward compatibility
using TokenType = jsonpath::TokenType;
using Token = jsonpath::Token;
using NodeType = jsonpath::NodeType;
using PathNode = jsonpath::PathNode;

/**
 * @brief JSONPath query result for mutable operations
 */
struct MutableQueryResult {
    std::vector<std::reference_wrapper<JsonStruct::JsonValue>> values;
    std::vector<std::string> paths;  // The actual paths where values were found
    
    bool empty() const { return values.empty(); }
    size_t size() const { return values.size(); }
    
    // Get first result if available
    std::optional<std::reference_wrapper<JsonStruct::JsonValue>> first() {
        return empty() ? std::nullopt : std::make_optional(values[0]);
    }
};

/**
 * @brief JSONPath query result
 */
struct QueryResult {
    std::vector<std::reference_wrapper<const JsonStruct::JsonValue>> values;
    std::vector<std::string> paths;  // The actual paths where values were found
    
    bool empty() const { return values.empty(); }
    size_t size() const { return values.size(); }
    
    // Get first result if available
    std::optional<std::reference_wrapper<const JsonStruct::JsonValue>> first() const {
        return empty() ? std::nullopt : std::make_optional(values[0]);
    }
};

/**
 * @brief JSONPath parser and evaluator (Main entry point)
 * 
 * This class coordinates the modular JSONPath components:
 * - Uses JsonPathTokenizer for tokenization
 * - Uses JsonPathParser for parsing
 * - Uses appropriate evaluators based on expression complexity
 */
class JsonPath {
private:
    std::string expression_;
    std::vector<PathNode> nodes_;
    
    // Parse and prepare the expression
    void parseExpression();

public:
    /**
     * @brief Construct JSONPath from expression string
     * @param expression JSONPath expression (e.g., "$.store.book[0].title")
     */
    explicit JsonPath(std::string expression);
    
    /**
     * @brief Get the original expression
     */
    const std::string& expression() const { return expression_; }
    
    /**
     * @brief Get parsed nodes (for advanced usage)
     */
    const std::vector<PathNode>& getNodes() const { return nodes_; }
    
    /**
     * @brief Evaluate JSONPath against a JSON value
     * @param root The root JSON value to query
     * @return Query results containing matching values and their paths
     */
    QueryResult evaluate(const JsonStruct::JsonValue& root) const;
    
    /**
     * @brief Evaluate JSONPath against a mutable JSON value
     * @param root The root JSON value to query
     * @return Query results containing matching mutable values and their paths
     */
    MutableQueryResult evaluateMutable(JsonStruct::JsonValue& root) const;
    
    /**
     * @brief Check if the path exists in the JSON value
     * @param root The root JSON value to check
     * @return True if at least one match is found
     */
    bool exists(const JsonStruct::JsonValue& root) const;
    
    /**
     * @brief Get first matching value
     * @param root The root JSON value to query  
     * @return First matching value if found
     */
    std::optional<std::reference_wrapper<const JsonStruct::JsonValue>> 
    selectFirst(const JsonStruct::JsonValue& root) const;
    
    /**
     * @brief Get all matching values
     * @param root The root JSON value to query
     * @return Vector of all matching values
     */
    std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>
    selectAll(const JsonStruct::JsonValue& root) const;
    
    /**
     * @brief Get first matching mutable value
     * @param root The root JSON value to query  
     * @return First matching mutable value if found
     */
    std::optional<std::reference_wrapper<JsonStruct::JsonValue>> 
    selectFirstMutable(JsonStruct::JsonValue& root) const;
    
    /**
     * @brief Get all matching mutable values
     * @param root The root JSON value to query
     * @return Vector of all matching mutable values
     */
    std::vector<std::reference_wrapper<JsonStruct::JsonValue>>
    selectAllMutable(JsonStruct::JsonValue& root) const;
    
    /**
     * @brief Validate JSONPath expression syntax
     * @param expression The expression to validate
     * @return True if syntax is valid
     */
    static bool isValidExpression(const std::string& expression);
    
    /**
     * @brief Parse and create JSONPath object
     * @param expression The expression to parse
     * @return JSONPath object if parsing succeeds
     * @throws JsonPathException if expression is invalid
     */
    static JsonPath parse(const std::string& expression);
};

/**
 * @brief JSONPath query exception
 */
class JsonPathException : public std::runtime_error {
public:
    JsonPathException(const std::string& message, size_t position)
        : std::runtime_error(message + " at position " + std::to_string(position))
        , position_(position) {}
    
    size_t position() const { return position_; }

private:
    size_t position_;
};

} // namespace jsonpath

/**
 * @brief JSONPath convenience functions in JsonValue
 */
namespace jsonvalue_jsonpath {

/**
 * @brief Evaluate JSONPath expression against mutable JSON value
 * @param root The JSON value to query
 * @param path_expression JSONPath expression
 * @return Mutable query results
 */
jsonpath::MutableQueryResult queryMutable(JsonStruct::JsonValue& root, const std::string& path_expression);

/**
 * @brief Select first mutable value matching JSONPath
 * @param root The JSON value to query
 * @param path_expression JSONPath expression
 * @return First matching mutable value if found
 */
std::optional<std::reference_wrapper<JsonStruct::JsonValue>>
selectFirstMutable(JsonStruct::JsonValue& root, const std::string& path_expression);

/**
 * @brief Select all mutable values matching JSONPath
 * @param root The JSON value to query
 * @param path_expression JSONPath expression
 * @return All matching mutable values
 */
std::vector<std::reference_wrapper<JsonStruct::JsonValue>>
selectAllMutable(JsonStruct::JsonValue& root, const std::string& path_expression);

/**
 * @brief Evaluate JSONPath expression against JSON value
 * @param root The JSON value to query
 * @param path_expression JSONPath expression
 * @return Query results
 */
jsonpath::QueryResult query(const JsonStruct::JsonValue& root, const std::string& path_expression);

/**
 * @brief Check if JSONPath exists in JSON value
 * @param root The JSON value to check
 * @param path_expression JSONPath expression
 * @return True if path exists
 */
bool exists(const JsonStruct::JsonValue& root, const std::string& path_expression);

/**
 * @brief Select first value matching JSONPath
 * @param root The JSON value to query
 * @param path_expression JSONPath expression
 * @return First matching value if found
 */
std::optional<std::reference_wrapper<const JsonStruct::JsonValue>>
selectFirst(const JsonStruct::JsonValue& root, const std::string& path_expression);

/**
 * @brief Select all values matching JSONPath
 * @param root The JSON value to query
 * @param path_expression JSONPath expression
 * @return All matching values
 */
std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>
selectAll(const JsonStruct::JsonValue& root, const std::string& path_expression);

} // namespace jsonvalue_jsonpath
