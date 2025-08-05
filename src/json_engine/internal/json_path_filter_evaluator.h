#pragma once

#include "json_path_parser.h"
#include <functional>
#include <vector>
#include <string>
#include <optional>

// Forward declaration
namespace JsonStruct {
    class JsonValue;
}

namespace jsonpath {

/**
 * @brief Filter evaluator - handles [?(...)] filter expressions
 * 
 * Handles:
 * - Basic conditions (@.property == value)
 * - Comparison operators (<, >, <=, >=, ==, !=)
 * - Logical operators (&&, ||)
 * - Nested filters (@.property[?(...)])
 * - Regular expressions (@.property =~ /pattern/)
 * - In operator (@.property in ['value1', 'value2'])
 * - Existence checks (@.property)
 */
class FilterEvaluator {
public:
    /**
     * @brief Check if nodes contain filters that this evaluator can handle
     * @param nodes The path nodes to check
     * @return True if any node is a filter
     */
    static bool canHandle(const std::vector<PathNode>& nodes);
    
    /**
     * @brief Evaluate filter path against const JSON value
     * @param root The root JSON value
     * @param nodes The parsed path nodes
     * @return Vector of const references to matching values and their paths
     */
    static std::pair<std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>, 
                     std::vector<std::string>>
    evaluate(const JsonStruct::JsonValue& root, const std::vector<PathNode>& nodes);
    
    /**
     * @brief Evaluate filter path against mutable JSON value
     * @param root The root JSON value
     * @param nodes The parsed path nodes
     * @return Vector of mutable references to matching values and their paths
     */
    static std::pair<std::vector<std::reference_wrapper<JsonStruct::JsonValue>>, 
                     std::vector<std::string>>
    evaluateMutable(JsonStruct::JsonValue& root, const std::vector<PathNode>& nodes);
    
    /**
     * @brief Evaluate filter against const JSON values
     * @param filter_expr The filter expression string
     * @param inputs Input values to filter
     * @param input_paths Paths corresponding to input values
     * @return Pair of filtered values and their paths
     */
    static std::pair<std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>, 
                     std::vector<std::string>>
    evaluate(const std::string& filter_expr,
             const std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& inputs,
             const std::vector<std::string>& input_paths);
    
    /**
     * @brief Evaluate filter against mutable JSON values
     * @param filter_expr The filter expression string
     * @param inputs Input values to filter
     * @param input_paths Paths corresponding to input values
     * @return Pair of filtered values and their paths
     */
    static std::pair<std::vector<std::reference_wrapper<JsonStruct::JsonValue>>, 
                     std::vector<std::string>>
    evaluateMutable(const std::string& filter_expr,
                    const std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& inputs,
                    const std::vector<std::string>& input_paths);

private:
    /**
     * @brief Evaluate a filter condition against a context value
     * @param condition The condition string
     * @param context The JSON value to evaluate against
     * @return True if condition matches
     */
    static bool evaluateFilterCondition(const std::string& condition, const JsonStruct::JsonValue& context);
    
    /**
     * @brief Get property value from context using JSONPath syntax
     * @param property The property path
     * @param context The JSON value to get property from
     * @return Pointer to property value or nullptr if not found
     */
    static const JsonStruct::JsonValue* getPropertyValue(const std::string& property, const JsonStruct::JsonValue& context);
    
    /**
     * @brief Handle logical AND expressions
     * @param expr The expression string
     * @param context The JSON value context
     * @return True if both sides are true
     */
    static bool handleLogicalAnd(const std::string& expr, const JsonStruct::JsonValue& context);
    
    /**
     * @brief Handle logical OR expressions
     * @param expr The expression string
     * @param context The JSON value context
     * @return True if either side is true
     */
    static bool handleLogicalOr(const std::string& expr, const JsonStruct::JsonValue& context);
    
    /**
     * @brief Handle nested filter expressions like @.friends[?(@.name == 'Bob')]
     * @param expr The expression string
     * @param context The JSON value context
     * @return True if nested filter has any matches
     */
    static bool handleNestedFilter(const std::string& expr, const JsonStruct::JsonValue& context);
    
    /**
     * @brief Handle comparison operations
     * @param expr The expression string
     * @param op The comparison operator
     * @param context The JSON value context
     * @return True if comparison succeeds
     */
    static bool handleComparison(const std::string& expr, const std::string& op, const JsonStruct::JsonValue& context);
    
    /**
     * @brief Handle regular expression matches
     * @param expr The expression string
     * @param context The JSON value context
     * @return True if regex matches
     */
    static bool handleRegexMatch(const std::string& expr, const JsonStruct::JsonValue& context);
    
    /**
     * @brief Handle 'in' operator expressions
     * @param expr The expression string
     * @param context The JSON value context
     * @return True if value is in the list
     */
    static bool handleInOperator(const std::string& expr, const JsonStruct::JsonValue& context);
    
    /**
     * @brief Handle method calls and calculations
     * @param expr The expression string
     * @param context The JSON value context
     * @return True if method call succeeds
     */
    static bool handleMethodCalculation(const std::string& expr, const JsonStruct::JsonValue& context);
    
    /**
     * @brief Handle existence checks
     * @param expr The expression string
     * @param context The JSON value context
     * @return True if property exists
     */
    static bool handleExistenceCheck(const std::string& expr, const JsonStruct::JsonValue& context);
};

} // namespace jsonpath
