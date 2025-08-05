#pragma once

#include "json_path_parser.h"
#include <functional>
#include <vector>
#include <string>

// Forward declaration
namespace JsonStruct {
    class JsonValue;
}

namespace jsonpath {

/**
 * @brief Advanced evaluator - handles recursive descent, union operations and other complex features
 * 
 * Handles:
 * - Recursive descent (..)
 * - Union operations (multiple paths separated by comma)
 * - Complex nested expressions
 */
class AdvancedEvaluator {
public:
    /**
     * @brief Check if nodes contain advanced features this evaluator can handle
     * @param nodes The parsed path nodes
     * @return True if contains recursive, union or other advanced features
     */
    static bool canHandle(const std::vector<PathNode>& nodes);
    
    /**
     * @brief Evaluate advanced path features against const JSON value
     * @param root The root JSON value
     * @param nodes The parsed path nodes
     * @return Pair of matching values and their paths
     */
    static std::pair<std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>, 
                     std::vector<std::string>>
    evaluate(const JsonStruct::JsonValue& root, const std::vector<PathNode>& nodes);
    
    /**
     * @brief Evaluate advanced path features against mutable JSON value
     * @param root The root JSON value
     * @param nodes The parsed path nodes
     * @return Pair of matching values and their paths
     */
    static std::pair<std::vector<std::reference_wrapper<JsonStruct::JsonValue>>, 
                     std::vector<std::string>>
    evaluateMutable(JsonStruct::JsonValue& root, const std::vector<PathNode>& nodes);

private:
    template<typename ValueType>
    static void evaluateRecursive(const PathNode& node,
                                const std::vector<std::reference_wrapper<ValueType>>& inputs,
                                const std::vector<std::string>& input_paths,
                                std::vector<std::reference_wrapper<ValueType>>& outputs,
                                std::vector<std::string>& output_paths);
    
    template<typename ValueType>
    static void collectRecursive(ValueType& value, const std::string& base_path,
                               std::vector<std::reference_wrapper<ValueType>>& outputs,
                               std::vector<std::string>& output_paths);
    
    template<typename ValueType>
    static void collectRecursiveProperty(ValueType& value, const std::string& base_path,
                                        const std::string& target_property,
                                        std::vector<std::reference_wrapper<ValueType>>& outputs,
                                        std::vector<std::string>& output_paths);
    
    template<typename ValueType>
    static void evaluateUnion(const PathNode& node,
                            const std::vector<std::reference_wrapper<ValueType>>& inputs,
                            const std::vector<std::string>& input_paths,
                            std::vector<std::reference_wrapper<ValueType>>& outputs,
                            std::vector<std::string>& output_paths);
    
    // For union evaluation, we need to create temporary evaluators
    template<typename ValueType>
    static std::pair<std::vector<std::reference_wrapper<ValueType>>, std::vector<std::string>>
    evaluatePathExpression(ValueType& root, const std::string& expression);
};

} // namespace jsonpath
