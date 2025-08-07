#include "query_factory.h"

namespace JsonStruct {

LazyQueryGenerator QueryFactory::createGenerator(
    const JsonFilter& filter,
    const JsonValue& jsonValue, 
    const std::string& expression, 
    size_t maxResults
) {
    return LazyQueryGenerator(&filter, &jsonValue, expression, maxResults);
}

LazyQueryGenerator QueryFactory::createGenerator(
    const JsonFilter& filter,
    const JsonValue& jsonValue, 
    const FilterFunction& filterFunc
) {
    return LazyQueryGenerator(&filter, &jsonValue, filterFunc);
}

} // namespace JsonStruct
