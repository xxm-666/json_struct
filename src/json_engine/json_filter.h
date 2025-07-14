#pragma once

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <unordered_map>
#include <regex>
#include <memory>

// Unified JSONPath engine include
#include "json_path.h"

// Forward declaration to avoid circular dependency
namespace JsonStruct {
    class JsonValue;
}

namespace JsonStruct {

/**
 * @brief JsonFilter - Dedicated class for JSON data filtering and querying
 *
 * This class provides powerful JSON data filtering capabilities, supporting:
 * - JSONPath queries (RFC 9535)
 * - Custom filter functions
 * - Chainable query operations
 * - Batch queries and transformations
 *
 * Design principles:
 * - Single responsibility: dedicated to data filtering and querying
 * - Extensible: supports custom filters and query strategies
 * - High performance: optimized query algorithms and caching
 * - Usability: concise API and chainable calls
 */
class JsonFilter {
public:
    // Query options
    struct QueryOptions {
        bool caseSensitive = true;          // Case sensitive
        bool allowWildcard = true;          // Allow wildcards
        bool enableCaching = false;         // Enable query cache
        size_t maxResults = 0;              // Maximum result count (0 = unlimited)
        bool stopOnFirstMatch = false;      // Stop on first match
        bool includeArrayIndices = true;    // Include array indices in results

        // Advanced options
        bool recursiveDescentEnabled = true; // Enable recursive descent (..)
        bool slicingEnabled = true;          // Enable array slicing ([start:end:step])
        bool filterExpressionsEnabled = true; // Enable filter expressions ([?(@.price < 10)])
    };

    // Query result
    struct QueryResult {
        const JsonValue* value = nullptr;   // Pointer to the matched value
        std::string path;                   // Full JSONPath
        std::vector<std::string> pathTokens; // Path tokens
        size_t depth = 0;                   // Nesting depth

        // Additional info
        bool isArrayElement = false;        // Is array element
        size_t arrayIndex = 0;              // Array index if applicable
        const JsonValue* parent = nullptr;  // Parent node pointer

        // Constructors
        QueryResult() = default;
        QueryResult(const JsonValue* val, const std::string& p, size_t d = 0)
            : value(val), path(p), depth(d) {}

        // Convenient constructor from jsonpath::QueryResult
        static std::vector<QueryResult> fromJsonPathResult(const jsonpath::QueryResult& jpResult);

        bool isValid() const noexcept { return value != nullptr; }
    };

    // Custom filter function types
    using FilterFunction = std::function<bool(const JsonValue&, const std::string& path)>;
    using TransformFunction = std::function<JsonValue(const JsonValue&, const std::string& path)>;

    // Filter strategy enum
    enum class FilterStrategy {
        JSONPath,           // Use JSONPath expression
        CustomFunction,     // Use custom function
        Regex,              // Use regex to match path
        Composite           // Composite strategies
    };

public:
    // Constructor
    explicit JsonFilter(const QueryOptions& options = {});

    // Disable copy, allow move
    JsonFilter(const JsonFilter&) = delete;
    JsonFilter& operator=(const JsonFilter&) = delete;
    JsonFilter(JsonFilter&&) = default;
    JsonFilter& operator=(JsonFilter&&) = default;

    // === Basic query methods ===
    
    /**
     * @brief Check if the specified path exists
     * @param jsonValue JSON object to query
     * @param expression JSONPath expression
     * @return Whether a matching path exists
     */
    bool pathExists(const JsonValue& jsonValue, const std::string& expression) const;
    
    /**
     * @brief Select the first matching value
     * @param jsonValue JSON object to query
     * @param expression JSONPath expression
     * @return Pointer to the first matching value, nullptr if not found
     */
    const JsonValue* selectFirst(const JsonValue& jsonValue, const std::string& expression) const;
    
    /**
     * @brief Select all matching values
     * @param jsonValue JSON object to query
     * @param expression JSONPath expression
     * @return Vector of pointers to all matching values
     */
    std::vector<const JsonValue*> selectAll(const JsonValue& jsonValue, const std::string& expression) const;
    
    /**
     * @brief Select all matching values (returns copies)
     * @param jsonValue JSON object to query
     * @param expression JSONPath expression
     * @return Vector of copies of all matching values
     */
    std::vector<JsonValue> selectValues(const JsonValue& jsonValue, const std::string& expression) const;

    // === Advanced query methods ===
    
    /**
     * @brief Execute detailed query, returning results with path info
     * @param jsonValue JSON object to query
     * @param expression JSONPath expression
     * @return Detailed query results
     */
    std::vector<QueryResult> query(const JsonValue& jsonValue, const std::string& expression) const;
    
    /**
     * @brief Query using custom filter function
     * @param jsonValue JSON object to query
     * @param filter Custom filter function
     * @return Matching query results
     */
    std::vector<QueryResult> queryWithFilter(const JsonValue& jsonValue, const FilterFunction& filter) const;
    
    /**
     * @brief Query using regex to match path
     * @param jsonValue JSON object to query
     * @param pathPattern Regex pattern for path
     * @return Matching query results
     */
    std::vector<QueryResult> queryWithRegex(const JsonValue& jsonValue, const std::string& pathPattern) const;

    // === Chainable query methods ===
    
    /**
     * @brief Create chainable query builder
     * @param jsonValue JSON object to query
     * @return Query builder
     */
    class QueryBuilder;
    QueryBuilder from(const JsonValue& jsonValue) const;

    // === Batch operation methods ===
    
    /**
     * @brief Batch query multiple expressions
     * @param jsonValue JSON object to query
     * @param expressions List of JSONPath expressions
     * @return Query results for each expression
     */
    std::vector<std::vector<QueryResult>> batchQuery(
        const JsonValue& jsonValue, 
        const std::vector<std::string>& expressions) const;
    
    /**
     * @brief Apply transform function to query results
     * @param results Query results
     * @param transform Transform function
     * @return Transformed values
     */
    std::vector<JsonValue> transform(
        const std::vector<QueryResult>& results, 
        const TransformFunction& transform) const;

    // === Configuration methods ===
    
    /**
     * @brief Set query options
     * @param options New query options
     */
    void setOptions(const QueryOptions& options);
    
    /**
     * @brief Get current query options
     * @return Current query options
     */
    const QueryOptions& getOptions() const noexcept;
    
    /**
     * @brief Clear query cache
     */
    void clearCache();

    // === Static convenience methods ===
    
    /**
     * @brief Create default filter instance
     * @return Default configured filter
     */
    static JsonFilter createDefault();
    
    /**
     * @brief Create high performance filter instance
     * @return High performance configured filter
     */
    static JsonFilter createHighPerformance();
    
    /**
     * @brief Create strict mode filter instance
     * @return Strict mode configured filter
     */
    static JsonFilter createStrict();

    // === Predefined filters ===
    
    /**
     * @brief Predefined commonly used filter functions
     */
    struct Filters {
        // Type-based filters - use int to avoid forward declaration issues
        static FilterFunction byType(int type);  // Use integer value of JsonValue::Type
        static FilterFunction byString(const std::string& value, bool caseSensitive = true);
        static FilterFunction byNumber(double value, double tolerance = 1e-9);
        static FilterFunction byNumberRange(double min, double max);

        // Path-based filters
        static FilterFunction byDepth(size_t minDepth, size_t maxDepth = SIZE_MAX);
        static FilterFunction byPathPattern(const std::string& pattern);

        // Value-based filters
        static FilterFunction hasProperty(const std::string& property);
        static FilterFunction arraySize(size_t minSize, size_t maxSize = SIZE_MAX);
        static FilterFunction isEmpty();
        static FilterFunction isNotEmpty();
    };

    // === Chainable query builder ===
    
    /**
     * @brief Chainable query builder, supports fluent API
     */
    class QueryBuilder {
public:
    explicit QueryBuilder(const JsonFilter& filter, const JsonValue& jsonValue);

    // Chainable methods
    QueryBuilder& where(const std::string& expression);
    QueryBuilder& where(const FilterFunction& filter);
    QueryBuilder& orderBy(const std::string& expression, bool ascending = true);
    QueryBuilder& groupBy(const std::string& expression);
    QueryBuilder& limit(size_t count);
    QueryBuilder& skip(size_t count);
    QueryBuilder& recursive();
    QueryBuilder& shallow();

    // Execution methods
    std::vector<QueryResult> execute() const;
    std::optional<QueryResult> first() const;
    std::vector<JsonValue> values() const;

    // Grouped results
    std::map<std::string, std::vector<QueryResult>> executeGrouped() const;

    // Aggregation methods
    size_t count() const;
    bool any() const;
    bool all(const FilterFunction& predicate) const;

private:
    const JsonFilter& filter_;
    const JsonValue& jsonValue_;
    std::vector<std::string> expressions_;
    std::vector<FilterFunction> customFilters_;
    std::string orderExpression_;
    bool orderAscending_ = true;
    std::string groupByExpression_;
    size_t limitCount_ = 0;
    size_t skipCount_ = 0;
    bool recursiveMode_ = false;
    };

private:
    // Query options
    QueryOptions options_;
    
    // Cache related
    mutable std::unordered_map<std::string, std::vector<QueryResult>> queryCache_;
    
    // Internal implementation methods
    std::vector<QueryResult> executeQuery(
        const JsonValue& jsonValue, 
        const std::string& expression,
        FilterStrategy strategy = FilterStrategy::JSONPath) const;
    
    // Unified JSONPath execution engine (using json_path.h)
    std::vector<QueryResult> executeJsonPathUnified(
        const JsonValue& jsonValue, 
        const std::string& expression) const;
    
    // Utility methods
    bool matchesFilter(const JsonValue& value, const std::string& path, const FilterFunction& filter) const;
    std::string buildPath(const std::string& basePath, const std::string& key) const;
    std::string buildArrayPath(const std::string& basePath, size_t index) const;
    bool isValidArrayIndex(const std::string& token) const;
    size_t parseArrayIndex(const std::string& token) const;
    
    // Cache management
    std::string buildCacheKey(const std::string& expression, FilterStrategy strategy) const;
    bool getCachedResult(const std::string& cacheKey, std::vector<QueryResult>& result) const;
    void setCachedResult(const std::string& cacheKey, const std::vector<QueryResult>& result) const;
};

// === Convenience functions ===

/**
 * @brief Global convenience functions for quick queries
 */
namespace query {
    
    // Quick query functions
    inline bool exists(const JsonValue& json, const std::string& path) {
        return JsonFilter::createDefault().pathExists(json, path);
    }
    
    inline const JsonValue* first(const JsonValue& json, const std::string& path) {
        return JsonFilter::createDefault().selectFirst(json, path);
    }
    
    inline std::vector<const JsonValue*> all(const JsonValue& json, const std::string& path) {
        return JsonFilter::createDefault().selectAll(json, path);
    }
    
    inline std::vector<JsonValue> values(const JsonValue& json, const std::string& path) {
        return JsonFilter::createDefault().selectValues(json, path);
    }
    
    // Chainable query entry point
    inline JsonFilter::QueryBuilder from(const JsonValue& json) {
        static JsonFilter defaultFilter = JsonFilter::createDefault();
        return defaultFilter.from(json);
    }
}

// Type convenience functions, avoid forward declaration issues
namespace filter_types {
    constexpr int Null = 0;
    constexpr int Bool = 1;
    constexpr int Number = 2;
    constexpr int String = 3;
    constexpr int Array = 4;
    constexpr int Object = 5;
}

} // namespace JsonStruct
