#pragma once

#include "lazy_query_generator.h"
#include "json_filter.h"

namespace JsonStruct {

/**
 * @brief Factory functions for creating enhanced query generators
 * 
 * These functions provide a way to create enhanced query generators
 * without circular dependencies in the class hierarchy.
 */
class QueryFactory {
public:
    /**
     * @brief Create query generator with JSONPath expression
     * @param filter The JsonFilter instance to use
     * @param jsonValue The JSON value to query
     * @param expression JSONPath expression with full feature support
     * @param maxResults Maximum number of results for optimization
     * @return Lazy query generator
     */
    static LazyQueryGenerator createGenerator(
        const JsonFilter& filter,
        const JsonValue& jsonValue, 
        const std::string& expression, 
        size_t maxResults = 0
    );

    /**
     * @brief Create query generator with filter function
     * @param filter The JsonFilter instance to use
     * @param jsonValue The JSON value to query
     * @param filterFunc Custom filter function
     * @return Lazy query generator
     */
    static LazyQueryGenerator createGenerator(
        const JsonFilter& filter,
        const JsonValue& jsonValue, 
        const FilterFunction& filterFunc
    );
};

} // namespace JsonStruct
