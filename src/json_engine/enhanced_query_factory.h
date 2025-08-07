#pragma once

#include "enhanced_lazy_query_generator.h"
#include "json_filter.h"

namespace JsonStruct {

/**
 * @brief Factory functions for creating enhanced query generators
 * 
 * These functions provide a way to create enhanced query generators
 * without circular dependencies in the class hierarchy.
 */
class EnhancedQueryFactory {
public:
    /**
     * @brief Create enhanced query generator with JSONPath expression
     * @param filter The JsonFilter instance to use
     * @param jsonValue The JSON value to query
     * @param expression JSONPath expression with full feature support
     * @param maxResults Maximum number of results for optimization
     * @return Enhanced lazy query generator
     */
    static EnhancedLazyQueryGenerator createGenerator(
        const JsonFilter& filter,
        const JsonValue& jsonValue, 
        const std::string& expression, 
        size_t maxResults = 0
    );

    /**
     * @brief Create enhanced query generator with filter function
     * @param filter The JsonFilter instance to use
     * @param jsonValue The JSON value to query
     * @param filterFunc Custom filter function
     * @return Enhanced lazy query generator
     */
    static EnhancedLazyQueryGenerator createGenerator(
        const JsonFilter& filter,
        const JsonValue& jsonValue, 
        const FilterFunction& filterFunc
    );
};

} // namespace JsonStruct
