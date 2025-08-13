#pragma once

#include "json_path_parser.h"
#include <functional>
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <json_engine/json_value.h>

namespace jsonpath {

// Method handler function type
using MethodHandler = std::function<std::optional<JsonStruct::JsonValue>(const JsonStruct::JsonValue&)>;

// Method call result structure
struct MethodCallResult {
    bool success;
    JsonStruct::JsonValue value;

    MethodCallResult() : success(false), value(JsonStruct::JsonValue()) {}
    MethodCallResult(const JsonStruct::JsonValue& val) : success(true), value(val) {}
};

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
 * - Method calls (@.property.method())
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

    /**
     * @brief Register a custom method handler
     * @param method_name Name of the method (e.g., "length", "max")
     * @param handler Function to handle the method call
     */
    static void registerMethod(const std::string& method_name, MethodHandler handler);

    /**
     * @brief Unregister a method handler
     * @param method_name Name of the method to remove
     */
    static void unregisterMethod(const std::string& method_name);

    /**
     * @brief Clear all registered methods (useful for testing)
     */
    static void clearMethods();

    /**
     * @brief Evaluate a filter condition against a context value
     * @param condition The condition string
     * @param context The JSON value to evaluate against
     * @return True if condition matches
     */
    static bool evaluateFilterCondition(const std::string& condition, const JsonStruct::JsonValue& context);
private:   
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
     * @brief Get the global method registry
     * @return Reference to the method registry map
     */
    static std::unordered_map<std::string, MethodHandler>& getMethodRegistry();

    /**
     * @brief Initialize built-in methods
     */
    static void initializeBuiltinMethods();

    /**
     * @brief Parse and execute method call
     * @param expr The expression containing method call
     * @param context The JSON value context
     * @return Method call result
     */
    static MethodCallResult executeMethodCall(const std::string& expr, const JsonStruct::JsonValue& context);

    /**
     * @brief Extract method name and property path from expression
     * @param expr The expression string
     * @return Pair of property path and method name, or empty if invalid
     */
    static std::pair<std::string, std::string> parseMethodCall(const std::string& expr);

    // Chain element for chained method calls
    enum class ChainElementType {
        PROPERTY,  // Property access like .prop
        METHOD     // Method call like .method()
    };

    struct ChainElement {
        ChainElementType type;
        std::string name;
        
        ChainElement(ChainElementType t, const std::string& n) : type(t), name(n) {}
    };

    /**
     * @brief Parse chained method calls and property accesses
     * @param expr The expression string (e.g., "@.prop1.method1().prop2.method2()")
     * @return Vector of chain elements, or empty if invalid
     */
    static std::vector<ChainElement> parseChainedMethodCalls(const std::string& expr);

    /**
     * @brief Execute chained method calls and property accesses
     * @param chain The parsed chain elements
     * @param context The JSON value context
     * @return Final result after executing the chain
     */
    static MethodCallResult executeChainedMethodCalls(const std::vector<ChainElement>& chain, const JsonStruct::JsonValue& context);

    // Built-in method handlers
    static std::optional<JsonStruct::JsonValue> lengthMethodHandler(const JsonStruct::JsonValue& value);
    static std::optional<JsonStruct::JsonValue> maxMethodHandler(const JsonStruct::JsonValue& value);
    static std::optional<JsonStruct::JsonValue> minMethodHandler(const JsonStruct::JsonValue& value);
    static std::optional<JsonStruct::JsonValue> sizeMethodHandler(const JsonStruct::JsonValue& value);

    /**
     * @brief Handle method calls and calculations (refactored)
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
