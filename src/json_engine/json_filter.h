#pragma once

#include <functional>
#include "json_value.h"
#include "json_path.h"
#include <functional>
#include <vector>
#include <stack>
#include <string>
#include <optional>
#include <memory>

namespace JsonStruct {

class LazyQueryGenerator;

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
     * @brief Create a JsonFilter instance with default options.
     * @return Default JsonFilter instance.
     * @note Recommended for quickly obtaining a standard filter.
     */
    static JsonFilter createDefault();

    /**
     * @brief Create a JsonFilter instance with custom options.
     * @param options Filter configuration options.
     * @return JsonFilter instance.
     * @note Options include case sensitivity, max depth, optimizations, etc.
     */
    static JsonFilter createWithOptions(const JsonFilterOptions& options);

    /**
     * @brief Constructor.
     * @param options Filter configuration options (optional).
     */
    explicit JsonFilter(const JsonFilterOptions& options = {});

    /**
     * @brief Query JSON data using a JSONPath expression.
     * @param jsonValue The JSON value to query.
     * @param expression JSONPath expression (e.g. "$.store.book[*].title").
     * @return Vector of QueryResult for all matches.
     * @throws std::runtime_error If the expression is invalid or query fails.
     * @note Supports standard JSONPath syntax.
     * @see queryGenerator()
     * @code
     * auto results = filter.query(json, "$.store.book[*].title");
     * for (const auto& r : results) { std::cout << r.path << std::endl; }
     * @endcode
     */
    std::vector<QueryResult> query(const JsonValue& jsonValue, const std::string& expression) const;

    /**
     * @brief Fast query using JSONPath expression (optimized for large arrays).
     * @param jsonValue The JSON value to query.
     * @param expression JSONPath expression.
     * @param maxResults Maximum number of results to return (0 for unlimited).
     * @return Vector of QueryResult for all matches.
     * @note Uses efficient traversal for large data sets.
     */
    std::vector<QueryResult> queryFast(const JsonValue& jsonValue, const std::string& expression, size_t maxResults = 0) const;

    /**
     * @brief Query JSON data using a custom filter function.
     * @param jsonValue The JSON value to query.
     * @param filter Filter function, returns true for match.
     * @return Vector of QueryResult for all matches.
     * @note Useful for complex custom filtering.
     */
    std::vector<QueryResult> query(const JsonValue& jsonValue, const FilterFunction& filter) const;

    /**
     * @brief Query the first match using a JSONPath expression.
     * @param jsonValue The JSON value to query.
     * @param expression JSONPath expression.
     * @return The first QueryResult if found, otherwise std::nullopt.
     * @note For use cases where only the first match is needed.
     */
    std::optional<QueryResult> queryFirst(const JsonValue& jsonValue, const std::string& expression) const;

    /**
     * @brief Check if a path exists in the JSON value.
     * @param jsonValue The JSON value to check.
     * @param expression JSONPath expression.
     * @return True if the path exists, false otherwise.
     * @note Equivalent to whether queryFirst() returns a value.
     */
    bool exists(const JsonValue& jsonValue, const std::string& expression) const;

    /**
     * @brief Count the number of matches for a JSONPath expression.
     * @param jsonValue The JSON value to query.
     * @param expression JSONPath expression.
     * @return Number of matches.
     * @note Fast way to count matches without retrieving all results.
     */
    size_t count(const JsonValue& jsonValue, const std::string& expression) const;

    // Backward compatibility methods (for JsonValue, etc.)

    /**
     * @brief Check if a path exists (legacy interface).
     * @param jsonValue The JSON value to check.
     * @param expression JSONPath expression.
     * @return True if the path exists, false otherwise.
     * @deprecated Use exists() instead.
     */
    bool pathExists(const JsonValue& jsonValue, const std::string& expression) const;

    /**
     * @brief Select the first matching value (legacy interface).
     * @param jsonValue The JSON value to query.
     * @param expression JSONPath expression.
     * @return Pointer to the first matching value, or nullptr if not found.
     * @deprecated Use queryFirst() instead.
     */
    const JsonValue* selectFirst(const JsonValue& jsonValue, const std::string& expression) const;

    /**
     * @brief Select all matching values (legacy interface).
     * @param jsonValue The JSON value to query.
     * @param expression JSONPath expression.
     * @return Vector of pointers to all matching values.
     * @deprecated Use query() instead.
     */
    std::vector<const JsonValue*> selectAll(const JsonValue& jsonValue, const std::string& expression) const;

    /**
     * @brief Select copies of all matching values (legacy interface).
     * @param jsonValue The JSON value to query.
     * @param expression JSONPath expression.
     * @return Vector of copies of all matching values.
     * @deprecated Use query() instead.
     */
    std::vector<JsonValue> selectValues(const JsonValue& jsonValue, const std::string& expression) const;

    // Path construction helpers

    /**
     * @brief Build a property path.
     * @param basePath The base path.
     * @param property The property name.
     * @return The full path string.
     * @note Used to generate paths like "$.a.b".
     */
    std::string buildPath(const std::string& basePath, const std::string& property) const;

    /**
     * @brief Build an array index path.
     * @param basePath The base path.
     * @param index The array index.
     * @return The full path string.
     * @note Used to generate paths like "$.a[0]".
     */
    std::string buildArrayPath(const std::string& basePath, size_t index) const;

    // Internal matching helper

    /**
     * @brief Check if a value matches the filter condition.
     * @param value The JSON value to check.
     * @param path The current path of the value.
     * @param filter The filter function.
     * @return True if matches, false otherwise.
     * @note Used for internal recursive traversal.
     */
    bool matchesFilter(const JsonValue& value, const std::string& path, const FilterFunction& filter) const;

    // Optimized array traversal

    /**
     * @brief Efficient traversal for large arrays.
     * @param jsonValue The JSON value to query.
     * @param expression JSONPath expression.
     * @param maxResults Maximum number of results.
     * @return Vector of QueryResult for all matches.
     * @note
     * Usage requirements:
     * - Intended for scenarios where the root node is a JSON array and the query expression targets array elements directly (e.g., "$.items[*]").
     * - For best performance, use when the array is large and only a subset of results is needed (early termination).
     * - Not suitable for deeply nested or non-array root queries; use query() or queryFast() in those cases.
     * - The function may bypass some general JSONPath features for speed, so ensure the expression is compatible.
     * - Automatically selects the optimal strategy internally, but caller should ensure input fits the above pattern.
     */
    std::optional<std::vector<QueryResult>> optimizedArrayTraversal(const JsonValue& jsonValue, const std::string& expression, size_t maxResults) const;

    /**
     * @brief Modern lazy query generator (recommended).
     * @param jsonValue The JSON value to query.
     * @param expression JSONPath expression.
     * @param maxResults Maximum number of results for optimization.
     * @return High-performance lazy query generator.
     * @note Supports pause/resume, early termination, memory efficiency.
     * @see LazyQueryGenerator
     * @code
     * auto gen = filter.queryGenerator(json, "$.a[*]");
     * while (gen.hasNext()) { auto r = gen.next(); ... }
     * @endcode
     */
    LazyQueryGenerator queryGenerator(const JsonValue& jsonValue, const std::string& expression, size_t maxResults = 0) const;

    /**
     * @brief Lazy query generator based on filter function.
     * @param jsonValue The JSON value to query.
     * @param filter Custom filter function.
     * @return Lazy query generator.
     * @note Suitable for complex custom scenarios.
     */
    LazyQueryGenerator queryGenerator(const JsonValue& jsonValue, const FilterFunction& filter) const;

private:
    JsonFilterOptions options_;
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
