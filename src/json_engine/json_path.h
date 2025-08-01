#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <variant>
#include <regex>
#include <functional>
#include <algorithm>
#include <stdexcept>

// Forward declaration
namespace JsonStruct {
    class JsonValue;
}

/**
 * @brief JSONPath query language implementation
 * 
 * Supports a subset of JSONPath expressions:
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
    int slice_end = -1;             // For SLICE nodes (-1 = end)
    int slice_step = 1;             // For SLICE nodes with step
    std::string filter_expr;        // For FILTER nodes
    std::vector<int> union_indices; // For UNION index nodes like [0,2,4]
    std::vector<std::string> union_paths; // For UNION path expressions
    
    PathNode(NodeType t) : type(t) {}
    PathNode(NodeType t, std::string prop) : type(t), property(std::move(prop)) {}
    PathNode(NodeType t, int idx) : type(t), index(idx) {}
};

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
 * @brief JSONPath parser and evaluator
 */
class JsonPath {
private:
    std::string expression_;
    std::vector<PathNode> nodes_;
    
    // Tokenizer
    std::vector<Token> tokenize(std::string_view expr);
    
    // Parser
    void parseExpression(const std::vector<Token>& tokens);
    PathNode parseNode(const std::vector<Token>& tokens, size_t& pos);
    
    // Union expression parsing
    bool hasTopLevelComma(const std::string& expr);
    void parseUnionExpression(const std::string& expr);
    
    // Evaluator helpers for mutable operations
    void evaluateNodeMutable(const PathNode& node, 
                             const std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& inputs,
                             const std::vector<std::string>& input_paths,
                             std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& outputs,
                             std::vector<std::string>& output_paths) const;
    
    void evaluatePropertyMutable(const std::string& property,
                                const std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& inputs,
                                const std::vector<std::string>& input_paths,
                                std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& outputs,
                                std::vector<std::string>& output_paths) const;
    
    void evaluateIndexMutable(int index,
                             const std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& inputs,
                             const std::vector<std::string>& input_paths,
                             std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& outputs,
                             std::vector<std::string>& output_paths) const;
    
    void evaluateSliceMutable(int start, int end, int step,
                             const std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& inputs,
                             const std::vector<std::string>& input_paths,
                             std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& outputs,
                             std::vector<std::string>& output_paths) const;
    
    void evaluateWildcardMutable(const std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& inputs,
                                const std::vector<std::string>& input_paths,
                                std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& outputs,
                                std::vector<std::string>& output_paths) const;
    
    void evaluateRecursiveMutable(const PathNode& node,
                                 const std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& inputs,
                                 const std::vector<std::string>& input_paths,
                                 std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& outputs,
                                 std::vector<std::string>& output_paths) const;
    
    void collectRecursiveMutable(JsonStruct::JsonValue& value, const std::string& base_path,
                                std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& outputs,
                                std::vector<std::string>& output_paths) const;
    
    void collectRecursivePropertyMutable(JsonStruct::JsonValue& value, const std::string& base_path,
                                        const std::string& target_property,
                                        std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& outputs,
                                        std::vector<std::string>& output_paths) const;
    
    void evaluateFilterMutable(const std::string& filter_expr,
                              const std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& inputs,
                              const std::vector<std::string>& input_paths,
                              std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& outputs,
                              std::vector<std::string>& output_paths) const;
    
    void evaluateUnionMutable(const PathNode& node,
                             const std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& inputs,
                             const std::vector<std::string>& input_paths,
                             std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& outputs,
                             std::vector<std::string>& output_paths) const;

    // Evaluator helpers
    void evaluateNode(const PathNode& node, 
                      const std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& inputs,
                      const std::vector<std::string>& input_paths,
                      std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& outputs,
                      std::vector<std::string>& output_paths) const;
    
    void evaluateProperty(const std::string& property,
                         const std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& inputs,
                         const std::vector<std::string>& input_paths,
                         std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& outputs,
                         std::vector<std::string>& output_paths) const;
    
    void evaluateIndex(int index,
                       const std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& inputs,
                       const std::vector<std::string>& input_paths,
                       std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& outputs,
                       std::vector<std::string>& output_paths) const;
    
    void evaluateSlice(int start, int end, int step,
                       const std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& inputs,
                       const std::vector<std::string>& input_paths,
                       std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& outputs,
                       std::vector<std::string>& output_paths) const;
    
    void evaluateWildcard(const std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& inputs,
                          const std::vector<std::string>& input_paths,
                          std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& outputs,
                          std::vector<std::string>& output_paths) const;
    
    void evaluateRecursive(const PathNode& node,
                           const std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& inputs,
                           const std::vector<std::string>& input_paths,
                           std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& outputs,
                           std::vector<std::string>& output_paths) const;
    
    void collectRecursive(const JsonStruct::JsonValue& value, const std::string& base_path,
                          std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& outputs,
                          std::vector<std::string>& output_paths) const;
    
    void collectRecursiveProperty(const JsonStruct::JsonValue& value, const std::string& base_path,
                                  const std::string& target_property,
                                  std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& outputs,
                                  std::vector<std::string>& output_paths) const;
    
    void evaluateFilter(const std::string& filter_expr,
                        const std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& inputs,
                        const std::vector<std::string>& input_paths,
                        std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& outputs,
                        std::vector<std::string>& output_paths) const;
    
    void evaluateUnion(const PathNode& node,
                       const std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& inputs,
                       const std::vector<std::string>& input_paths,
                       std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& outputs,
                       std::vector<std::string>& output_paths) const;
    
    bool evaluateFilterCondition(const std::string& condition, const JsonStruct::JsonValue& context) const;
    bool evaluateBasicFilterCondition(const std::string& condition, const JsonStruct::JsonValue& context) const;
    std::optional<bool> parseNestedFilterExpression(const std::string& condition, const JsonStruct::JsonValue& context) const;
    bool filterKey_RegexIn(const std::string& condition, const JsonStruct::JsonValue& context) const;
	bool filterKey_RegexRegex(const std::string& condition, const JsonStruct::JsonValue& context) const;
	bool filterKey_ValueCalculate(const std::string& method,
                                    const std::string& operate,
		                            const std::string& value,
                                    const JsonStruct::JsonValue& propValue) const;
	bool filterKey_RegexSingleValue(const std::string& op, const std::string& opValue, const JsonStruct::JsonValue& value) const;
    // Utility functions
    static bool isValidIdentifier(char c);
    static std::string escapeJsonPointerToken(const std::string& token);

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
     * @brief Evaluate JSONPath against a mutable JSON value
     * @param root The root JSON value to query
     * @return Query results containing matching mutable values and their paths
     */
    MutableQueryResult evaluateMutable(JsonStruct::JsonValue& root) const;
    
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
     * @brief Evaluate JSONPath against a JSON value
     * @param root The root JSON value to query
     * @return Query results containing matching values and their paths
     */
    QueryResult evaluate(const JsonStruct::JsonValue& root) const;
    
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
     * @brief Validate JSONPath expression syntax
     * @param expression The expression to validate
     * @return True if syntax is valid
     */
    static bool isValidExpression(const std::string& expression);
    
    /**
     * @brief Parse and create JSONPath object
     * @param expression The expression to parse
     * @return JSONPath object if parsing succeeds
     * @throws std::invalid_argument if expression is invalid
     */
    static JsonPath parse(const std::string& expression);
    
    // 懒加载支持API
    /**
     * @brief Get parsed nodes for lazy evaluation
     * @return Reference to the parsed path nodes
     */
    const std::vector<PathNode>& getNodes() const { return nodes_; }
    
    /**
     * @brief Evaluate a single node against an input value (for lazy evaluation)
     * @param node The path node to evaluate
     * @param input Input value to process
     * @param input_path Path of the input value
     * @param outputs Vector to store output values
     * @return True if any results were found
     */
    bool evaluateSingleNode(const PathNode& node,
                           const JsonStruct::JsonValue* input, 
                           const std::string& input_path,
                           std::vector<std::pair<const JsonStruct::JsonValue*, std::string>>& outputs) const;
};

/**
 * @brief JSONPath query exception
 */
class JsonPathException : public std::runtime_error {
public:
    JsonPathException(const std::string& message, size_t position = 0)
        : std::runtime_error(message), position_(position) {}
    
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
