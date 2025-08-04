#include "json_path_advanced_evaluator.h"
#include "json_path_tokenizer.h"
#include "json_path_parser.h"
#include "json_path_simple_evaluator.h"
#include "json_path_filter_evaluator.h"
#include "json_value.h"

namespace jsonpath {

bool AdvancedEvaluator::canHandle(const std::vector<PathNode>& nodes) {
    for (const auto& node : nodes) {
        switch (node.type) {
            case NodeType::RECURSIVE:
            case NodeType::UNION:
            case NodeType::FILTER:
                return true; // These are advanced features
            default:
                break;
        }
    }
    return false;
}

std::pair<std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>, 
          std::vector<std::string>>
AdvancedEvaluator::evaluate(const JsonStruct::JsonValue& root, const std::vector<PathNode>& nodes) {
    std::vector<std::reference_wrapper<const JsonStruct::JsonValue>> current_values;
    std::vector<std::string> current_paths;
    
    // Start with root
    current_values.emplace_back(std::cref(root));
    current_paths.emplace_back("$");
    
    // Process each node
    for (size_t i = 1; i < nodes.size(); ++i) { // Skip ROOT node
        std::vector<std::reference_wrapper<const JsonStruct::JsonValue>> next_values;
        std::vector<std::string> next_paths;
        
        const PathNode& node = nodes[i];
        
        switch (node.type) {
            case NodeType::RECURSIVE:
                evaluateRecursive<const JsonStruct::JsonValue>(node, current_values, current_paths, next_values, next_paths);
                break;
                
            case NodeType::UNION:
                evaluateUnion<const JsonStruct::JsonValue>(node, current_values, current_paths, next_values, next_paths);
                break;
                
            case NodeType::FILTER:
                {
                    // For array filtering, we need to expand array elements and apply filter to each element
                    std::vector<std::reference_wrapper<const JsonStruct::JsonValue>> filter_inputs;
                    std::vector<std::string> filter_paths;
                    
                    for (size_t i = 0; i < current_values.size(); ++i) {
                        const auto& value = current_values[i].get();
                        const std::string& path = current_paths[i];
                        
                        if (value.isArray()) {
                            if (const auto& arr_opt = value.toArray()) {
                                const auto& arr = arr_opt->get();
                                for (size_t j = 0; j < arr.size(); ++j) {
                                    filter_inputs.emplace_back(arr[j]);
                                    filter_paths.push_back(path + "[" + std::to_string(j) + "]");
                                }
                            }
                        } else {
                            // For non-arrays, pass the value directly
                            filter_inputs.emplace_back(value);
                            filter_paths.push_back(path);
                        }
                    }
                    
                    auto result = FilterEvaluator::evaluate(node.filter_expr, filter_inputs, filter_paths);
                    next_values = std::move(result.first);
                    next_paths = std::move(result.second);
                }
                break;
                
            default:
                // Use simple evaluator for basic nodes
                SimplePathEvaluator::evaluateNode<const JsonStruct::JsonValue>(node, current_values, current_paths, next_values, next_paths);
                break;
        }
        
        current_values = std::move(next_values);
        current_paths = std::move(next_paths);
    }
    
    return {current_values, current_paths};
}

std::pair<std::vector<std::reference_wrapper<JsonStruct::JsonValue>>, 
          std::vector<std::string>>
AdvancedEvaluator::evaluateMutable(JsonStruct::JsonValue& root, const std::vector<PathNode>& nodes) {
    std::vector<std::reference_wrapper<JsonStruct::JsonValue>> current_values;
    std::vector<std::string> current_paths;
    
    // Start with root
    current_values.emplace_back(std::ref(root));
    current_paths.emplace_back("$");
    
    for (size_t i = 1; i < nodes.size(); ++i) { // Skip ROOT node
        std::vector<std::reference_wrapper<JsonStruct::JsonValue>> next_values;
        std::vector<std::string> next_paths;
        
        const PathNode& node = nodes[i];
        
        switch (node.type) {
            case NodeType::RECURSIVE:
                evaluateRecursive<JsonStruct::JsonValue>(node, current_values, current_paths, next_values, next_paths);
                break;
                
            case NodeType::UNION:
                evaluateUnion<JsonStruct::JsonValue>(node, current_values, current_paths, next_values, next_paths);
                break;
                
            case NodeType::FILTER:
                {
                    // For array filtering, we need to expand array elements and apply filter to each element
                    std::vector<std::reference_wrapper<JsonStruct::JsonValue>> filter_inputs;
                    std::vector<std::string> filter_paths;
                    
                    for (size_t i = 0; i < current_values.size(); ++i) {
                        auto& value = current_values[i].get();
                        const std::string& path = current_paths[i];
                        
                        if (auto& arr_opt = value.toArray()) {
                            // Expand array elements for filtering
                            auto& arr = arr_opt->get();
                            for (size_t j = 0; j < arr.size(); ++j) {
                                filter_inputs.emplace_back(arr[j]);
                                filter_paths.push_back(path + "[" + std::to_string(j) + "]");
                            }
                        } else {
                            // For non-arrays, pass the value directly
                            filter_inputs.emplace_back(value);
                            filter_paths.push_back(path);
                        }
                    }
                    
                    auto result = FilterEvaluator::evaluateMutable(node.filter_expr, filter_inputs, filter_paths);
                    next_values = std::move(result.first);
                    next_paths = std::move(result.second);
                }
                break;
                
            default:
                // Use simple evaluator for basic nodes
                SimplePathEvaluator::evaluateNode<JsonStruct::JsonValue>(node, current_values, current_paths, next_values, next_paths);
                break;
        }
        
        current_values = std::move(next_values);
        current_paths = std::move(next_paths);
    }
    
    return {current_values, current_paths};
}

template<typename ValueType>
void AdvancedEvaluator::evaluateRecursive(const PathNode& node,
                                         const std::vector<std::reference_wrapper<ValueType>>& inputs,
                                         const std::vector<std::string>& input_paths,
                                         std::vector<std::reference_wrapper<ValueType>>& outputs,
                                         std::vector<std::string>& output_paths) {
    for (size_t i = 0; i < inputs.size(); ++i) {
        auto& input = inputs[i].get();
        const std::string& input_path = input_paths[i];
        
        if (node.property.empty()) {
            // General recursive descent (..)
            collectRecursive<ValueType>(input, input_path, outputs, output_paths);
        } else {
            // Recursive property search (..property)
            collectRecursiveProperty<ValueType>(input, input_path, node.property, outputs, output_paths);
        }
    }
}

template<typename ValueType>
void AdvancedEvaluator::collectRecursive(ValueType& value, const std::string& base_path,
                                        std::vector<std::reference_wrapper<ValueType>>& outputs,
                                        std::vector<std::string>& output_paths) {
    // Add current value
    outputs.emplace_back(std::ref(value));
    output_paths.emplace_back(base_path);
    
    // Recursively collect from children
    if (value.isObject()) {
        if (const auto* obj = value.getObject()) {
            for (const auto& [key, child_value] : *obj) {
                std::string child_path = base_path + "." + key;
                collectRecursive<ValueType>(const_cast<ValueType&>(child_value), child_path, outputs, output_paths);
            }
        }
    } else if (value.isArray()) {
        if(const auto& arr_opt = value.toArray()) {
            const auto& arr = arr_opt->get();
            for (size_t idx = 0; idx < arr.size(); ++idx) {
                std::string child_path = base_path + "[" + std::to_string(idx) + "]";
                collectRecursive<ValueType>(const_cast<ValueType&>(arr[idx]), child_path, outputs, output_paths);
            }
        }
    }
}

template<typename ValueType>
void AdvancedEvaluator::collectRecursiveProperty(ValueType& value, const std::string& base_path,
                                                const std::string& target_property,
                                                std::vector<std::reference_wrapper<ValueType>>& outputs,
                                                std::vector<std::string>& output_paths) {
    // Check if current value has the target property
    if (value.isObject() && value.contains(target_property)) {
        outputs.emplace_back(std::ref(const_cast<ValueType&>(value[target_property])));
        output_paths.emplace_back(base_path + "." + target_property);
    }
    
    // Recursively search in children
    if (value.isObject()) {
        if (const auto* obj = value.getObject()) {
            for (const auto& [key, child_value] : *obj) {
                std::string child_path = base_path + "." + key;
                collectRecursiveProperty<ValueType>(const_cast<ValueType&>(child_value), child_path, target_property, outputs, output_paths);
            }
        }
    } else if (value.isArray()) {
        if (const auto& arr_opt = value.toArray()) {
            const auto &arr = arr_opt->get();
            for (size_t idx = 0; idx < arr.size(); ++idx) {
                std::string child_path = base_path + "[" + std::to_string(idx) + "]";
                collectRecursiveProperty<ValueType>(const_cast<ValueType&>(arr[idx]), child_path, target_property, outputs, output_paths);
            }
        }
    }
}

template<typename ValueType>
void AdvancedEvaluator::evaluateUnion(const PathNode& node,
                                     const std::vector<std::reference_wrapper<ValueType>>& inputs,
                                     const std::vector<std::string>& input_paths,
                                     std::vector<std::reference_wrapper<ValueType>>& outputs,
                                     std::vector<std::string>& output_paths) {
    // Handle union paths (comma-separated JSONPath expressions)
    if (!node.union_paths.empty()) {
        // For each input, evaluate all union paths
        for (size_t i = 0; i < inputs.size(); ++i) {
            auto& input = inputs[i].get();
            
            for (const std::string& path_expr : node.union_paths) {
                auto result = evaluatePathExpression<ValueType>(input, path_expr);
                
                // Add results to outputs
                for (auto& value_ref : result.first) {
                    outputs.emplace_back(value_ref);
                }
                for (const auto& path : result.second) {
                    output_paths.emplace_back(path);
                }
            }
        }
    }
    
    // Handle union indices (multiple array indices like [0,2,4])
    if (!node.union_indices.empty()) {
        for (size_t i = 0; i < inputs.size(); ++i) {
            const auto& input = inputs[i].get();
            const std::string& input_path = input_paths[i];
            
            if (const auto& arr_opt = input.toArray()) {
                const auto& arr = arr_opt->get();
                for (int idx : node.union_indices) {
                    // Handle negative indices
                    int actual_idx = idx;
                    if (actual_idx < 0) {
                        actual_idx = static_cast<int>(arr.size()) + actual_idx;
                    }
                        
                    if (actual_idx >= 0 && static_cast<size_t>(actual_idx) < arr.size()) {
                        outputs.emplace_back(std::ref(const_cast<ValueType&>(arr[actual_idx])));
                        output_paths.emplace_back(input_path + "[" + std::to_string(idx) + "]");
                    }
                }
            }
        }
    }
}

template<typename ValueType>
std::pair<std::vector<std::reference_wrapper<ValueType>>, std::vector<std::string>>
AdvancedEvaluator::evaluatePathExpression(ValueType& root, const std::string& expression) {
    try {
        // Tokenize and parse the expression
        auto tokens = JsonPathTokenizer::tokenize(expression);
        auto nodes = JsonPathParser::parse(tokens);
        
        // Use appropriate evaluator based on complexity
        if (SimplePathEvaluator::canHandle(nodes)) {
            if constexpr (std::is_const_v<ValueType>) {
                return SimplePathEvaluator::evaluate(root, nodes);
            } else {
                return SimplePathEvaluator::evaluateMutable(root, nodes);
            }
        } else {
            if constexpr (std::is_const_v<ValueType>) {
                return AdvancedEvaluator::evaluate(root, nodes);
            } else {
                return AdvancedEvaluator::evaluateMutable(root, nodes);
            }
        }
    } catch (const std::exception&) {
        // Return empty result on parse error
        return {{}, {}};
    }
}

// Explicit template instantiations
template void AdvancedEvaluator::evaluateRecursive<const JsonStruct::JsonValue>(
    const PathNode& node,
    const std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& inputs,
    const std::vector<std::string>& input_paths,
    std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& outputs,
    std::vector<std::string>& output_paths);

template void AdvancedEvaluator::evaluateRecursive<JsonStruct::JsonValue>(
    const PathNode& node,
    const std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& inputs,
    const std::vector<std::string>& input_paths,
    std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& outputs,
    std::vector<std::string>& output_paths);

template void AdvancedEvaluator::evaluateUnion<const JsonStruct::JsonValue>(
    const PathNode& node,
    const std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& inputs,
    const std::vector<std::string>& input_paths,
    std::vector<std::reference_wrapper<const JsonStruct::JsonValue>>& outputs,
    std::vector<std::string>& output_paths);

template void AdvancedEvaluator::evaluateUnion<JsonStruct::JsonValue>(
    const PathNode& node,
    const std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& inputs,
    const std::vector<std::string>& input_paths,
    std::vector<std::reference_wrapper<JsonStruct::JsonValue>>& outputs,
    std::vector<std::string>& output_paths);

} // namespace jsonpath
