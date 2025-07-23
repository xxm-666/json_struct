#pragma once

#include "json_value.h"
#include "json_path.h"
#include <functional>
#include <vector>
#include <stack>
#include <string>
#include <optional>
#include <memory>

/**
 * @brief JSON filter and query generator
 *
 * JsonFilter provides powerful JSON data query and filtering capabilities, supporting:
 * - JSONPath expression queries
 * - Custom filter functions
 * - Lazy query generator (performance optimization)
 * - Path construction and data extraction
 */

namespace JsonStruct {

/**
 * @brief Query result structure
 */
struct QueryResult {
    const JsonValue* value;
    std::string path;
    size_t depth;

    QueryResult() = default;
    QueryResult(const JsonValue* val, const std::string& p, size_t d)
        : value(val), path(p), depth(d) {}
};

/**
 * @brief Filter function type definition
 * @param value The JSON value to check
 * @param path The path of the value in the JSON document
 * @return Returns true if the value matches the filter condition
 */
using FilterFunction = std::function<bool(const JsonValue& value, const std::string& path)>;

/**
 * @brief JsonFilter options configuration
 */
struct JsonFilterOptions {
    bool includeNulls = false;          // Whether to include null values
    bool caseSensitive = true;          // Whether path matching is case sensitive
    size_t maxDepth = 100;              // Maximum recursion depth
    bool enableOptimizations = true;    // Whether to enable performance optimizations
};

/**
 * @brief JSON filter and query processor
 */
class JsonFilter {
public:
    /**
     * @brief Create a default JsonFilter instance
     */
    static JsonFilter createDefault();

    /**
     * @brief Create a JsonFilter instance with options
     */
    static JsonFilter createWithOptions(const JsonFilterOptions& options);

    /**
     * @brief Constructor
     */
    explicit JsonFilter(const JsonFilterOptions& options = {});

    /**
     * @brief Query JSON data using a JSONPath expression
     * @param jsonValue The JSON value to query
     * @param expression JSONPath expression (e.g. "$.store.book[*].title")
     * @return Vector of query results
     */
    std::vector<QueryResult> query(const JsonValue& jsonValue, const std::string& expression) const;

    /**
     * @brief Query JSON data using a JSONPath expression (fast path optimization version)
     * @param jsonValue The JSON value to query
     * @param expression JSONPath expression
     * @param maxResults Maximum number of results, 0 means unlimited
     * @return Vector of query results
     */
    std::vector<QueryResult> queryFast(const JsonValue& jsonValue, const std::string& expression, size_t maxResults = 0) const;

    /**
     * @brief Query JSON data using a custom filter function
     * @param jsonValue The JSON value to query
     * @param filter Filter function
     * @return Vector of query results
     */
    std::vector<QueryResult> query(const JsonValue& jsonValue, const FilterFunction& filter) const;

    /**
     * @brief Find the first match
     * @param jsonValue The JSON value to query
     * @param expression JSONPath expression
     * @return The first matching result, or nullopt if not found
     */
    std::optional<QueryResult> queryFirst(const JsonValue& jsonValue, const std::string& expression) const;

    /**
     * @brief Check if a path exists
     * @param jsonValue The JSON value to check
     * @param expression JSONPath expression
     * @return Returns true if the path exists
     */
    bool exists(const JsonValue& jsonValue, const std::string& expression) const;

    /**
     * @brief Count the number of matches
     * @param jsonValue The JSON value to query
     * @param expression JSONPath expression
     * @return Number of matches
     */
    size_t count(const JsonValue& jsonValue, const std::string& expression) const;

    // Backward compatibility methods (used by JsonValue)
    /**
     * @brief Check if a path exists (backward compatibility)
     */
    bool pathExists(const JsonValue& jsonValue, const std::string& expression) const;
    
    /**
     * @brief Select the first matching value (backward compatibility)
     */
    const JsonValue* selectFirst(const JsonValue& jsonValue, const std::string& expression) const;
    
    /**
     * @brief Select all matching values (backward compatibility)
     */
    std::vector<const JsonValue*> selectAll(const JsonValue& jsonValue, const std::string& expression) const;
    
    /**
     * @brief Select all matching value copies (backward compatibility)
     */
    std::vector<JsonValue> selectValues(const JsonValue& jsonValue, const std::string& expression) const;

    // Path construction helper functions
    std::string buildPath(const std::string& basePath, const std::string& property) const;
    std::string buildArrayPath(const std::string& basePath, size_t index) const;
    
    // Internal matching function
    bool matchesFilter(const JsonValue& value, const std::string& path, const FilterFunction& filter) const;
    
    // Fast path optimization helper
    std::vector<QueryResult> optimizedArrayTraversal(const JsonValue& jsonValue, const std::string& expression, size_t maxResults) const;

    /**
     * @brief Lazy query generator class
     *
     * This class implements true lazy evaluation for JSONPath queries, based on the proven implementation
     * from LazyJsonPathIterator. It provides on-demand calculation with full state preservation,
     * achieving significant performance improvements for partial result scenarios.
     *
     * Features:
     * - True lazy evaluation: calculates one result per next() call
     * - Complete state preservation: supports pause/resume at any point  
     * - Memory efficient: no pre-loading of results
     * - Early termination: stops computation when iteration stops
     * - Reuses json_path module for parsing and validation
     *
     * @example
     * auto generator = filter.queryGenerator(json, "$.store.book[*].title");
     * while (generator.hasNext()) {
     *     auto result = generator.next();
     *     if (someCondition) break; // Early termination saves computation
     * }
     */
public:
    class LazyQueryGenerator {
    public:
        // Constructor for JSONPath expressions
        LazyQueryGenerator(const JsonFilter* filter, const JsonValue* root, const std::string& expression);

        // Constructor for FilterFunction (backward compatibility)
        LazyQueryGenerator(const JsonFilter* filter, const JsonValue* root, FilterFunction func);

        // Constructor for optimized lazy loading with early termination hints
        LazyQueryGenerator(const JsonFilter* filter, const JsonValue* root, const std::string& expression, size_t maxResults);

        // Check if there is a next item
        bool hasNext() const;

        // Get the next result with true lazy evaluation
        QueryResult next();

        // Get multiple results at once for better performance
        std::vector<QueryResult> nextBatch(size_t maxCount = 10);

    private:
        // Enhanced frame structure for stateful lazy evaluation
        struct Frame {
            const JsonValue* value;
            std::string path;
            size_t nodeIndex = 0;
            
            // Recursive descent state management
            enum class RecursiveState { None, SearchingSelf, SearchingChildren };
            RecursiveState recursiveState = RecursiveState::None;
            std::string recursiveProperty;
            
            // Children storage for recursive processing (consistent with json_lazy_query.h)
            std::vector<std::pair<std::string, const JsonValue*>> children;
            
            // Optimized child iteration state (iterator-based)
            size_t childIndex = 0;
            
            // Array iteration state (for slice operations)
            size_t arrayIndex = 0;
            size_t arraySize = 0;
            
            Frame(const JsonValue* v, const std::string& p, size_t ni) 
                : value(v), path(p), nodeIndex(ni) {}
        };

        const JsonFilter* filter_;
        const JsonValue* root_;
        std::string expression_;
        FilterFunction filterFunc_;
        bool useFilterFunc_ = false;
        bool initialized_ = false;
        
        // JSONPath components (reusing json_path module)
        std::unique_ptr<jsonpath::JsonPath> jsonPath_;
        std::vector<jsonpath::PathNode> nodes_;
        
        // Lazy evaluation state
        std::stack<Frame> stack_;
        std::optional<QueryResult> current_;
        
        // Optimization hints
        size_t maxResults_ = 0;  // 0 means unlimited
        size_t resultCount_ = 0; // Current result count

        void initialize();
        void advance();
        
        // Node processing methods (adapted from LazyJsonPathIterator)
        bool processNode(Frame& frame, const jsonpath::PathNode& node);
        bool processProperty(Frame& frame, const std::string& property);
        bool processIndex(Frame& frame, int index);
        bool processSlice(Frame& frame, int start, int end);
        bool processWildcard(Frame& frame);
        bool processRecursive(Frame& frame, const std::string& property);
    };
    
    /**
     * @brief Modern lazy query generator (recommended)
     * @param jsonValue The JSON value to query
     * @param expression JSONPath expression
     * @param maxResults Maximum number of results, used for intelligent optimization strategy selection
     * @return High-performance lazy query generator
     *
     * This is the recommended lazy query interface, which automatically selects the optimal strategy:
     * - Small result sets (≤1000): use fast path optimization
     * - Large result sets (>1000): use true streaming processing
     */
    LazyQueryGenerator queryGenerator(const JsonValue& jsonValue, const std::string& expression, size_t maxResults = 0) const;

    /**
     * @brief Lazy query generator based on filter function
     * @param jsonValue The JSON value to query
     * @param filter Custom filter function
     * @return Lazy query generator
     */
    LazyQueryGenerator queryGenerator(const JsonValue& jsonValue, const FilterFunction& filter) const;

private:
    JsonFilterOptions options_;
    
    // Internal implementation functions
    std::vector<QueryResult> queryWithFilterFunction(const JsonValue& jsonValue, const FilterFunction& filter) const;
    std::vector<QueryResult> queryWithJsonPath(const JsonValue& jsonValue, const std::string& expression) const;
    
    void collectMatches(const JsonValue& value, const std::string& currentPath, size_t depth,
                       const FilterFunction& filter, std::vector<QueryResult>& results) const;
};

// === Convenience functions ===

/**
 * @brief Create a filter function based on JSONPath
 * @param expression JSONPath expression
 * @return Filter function
 */
FilterFunction createJsonPathFilter(const std::string& expression);

/**
 * @brief Create a filter function based on property existence
 * @param propertyName Property name
 * @return Filter function
 */
FilterFunction createPropertyExistsFilter(const std::string& propertyName);

/**
 * @brief Create a filter function based on type
 * @param type JSON type to match
 * @return Filter function
 */
FilterFunction createTypeFilter(JsonValue::Type type);

} // namespace JsonStruct
