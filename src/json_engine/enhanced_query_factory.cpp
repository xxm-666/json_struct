#include "enhanced_query_factory.h"

namespace JsonStruct {

EnhancedLazyQueryGenerator EnhancedQueryFactory::createGenerator(
    const JsonFilter& filter,
    const JsonValue& jsonValue, 
    const std::string& expression, 
    size_t maxResults
) {
    return EnhancedLazyQueryGenerator(&filter, &jsonValue, expression, maxResults);
}

EnhancedLazyQueryGenerator EnhancedQueryFactory::createGenerator(
    const JsonFilter& filter,
    const JsonValue& jsonValue, 
    const FilterFunction& filterFunc
) {
    return EnhancedLazyQueryGenerator(&filter, &jsonValue, filterFunc);
}

} // namespace JsonStruct
