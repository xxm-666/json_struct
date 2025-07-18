#include "json_path.h"
#include "json_value.h"
#include <cctype>
#include <sstream>
#include <string_view>
#include <optional>
#include <regex>

using JsonValue = JsonStruct::JsonValue;

namespace jsonpath {

JsonPath::JsonPath(std::string expression) : expression_(std::move(expression)) {
    auto tokens = tokenize(expression_);
    parseExpression(tokens);
}

std::vector<Token> JsonPath::tokenize(std::string_view expr) {
    std::vector<Token> tokens;
    size_t i = 0;
    
    while (i < expr.length()) {
        char c = expr[i];
        
        // Skip whitespace
        if (std::isspace(c)) {
            ++i;
            continue;
        }
        
        switch (c) {
            case '$':
                tokens.emplace_back(TokenType::ROOT, "$", i);
                ++i;
                break;
                
            case '.':
                if (i + 1 < expr.length() && expr[i + 1] == '.') {
                    tokens.emplace_back(TokenType::RECURSIVE, "..", i);
                    i += 2;
                } else {
                    tokens.emplace_back(TokenType::DOT, ".", i);
                    ++i;
                }
                break;
                
            case '[':
                tokens.emplace_back(TokenType::BRACKET_OPEN, "[", i);
                ++i;
                break;
                
            case ']':
                tokens.emplace_back(TokenType::BRACKET_CLOSE, "]", i);
                ++i;
                break;
                
            case '*':
                tokens.emplace_back(TokenType::WILDCARD, "*", i);
                ++i;
                break;
                
            case ':':
                tokens.emplace_back(TokenType::SLICE, ":", i);
                ++i;
                break;
                
            case '?':
                tokens.emplace_back(TokenType::FILTER, "?", i);
                ++i;
                break;
                
            case ',':
                tokens.emplace_back(TokenType::COMMA, ",", i);
                ++i;
                break;
                
            case '\'':
            case '"': {
                // String literal
                size_t start = i;
                char quote = c;
                ++i;
                std::string value;
                
                while (i < expr.length() && expr[i] != quote) {
                    if (expr[i] == '\\' && i + 1 < expr.length()) {
                        ++i; // Skip escape character
                        char escaped = expr[i];
                        switch (escaped) {
                            case 'n': value += '\n'; break;
                            case 't': value += '\t'; break;
                            case 'r': value += '\r'; break;
                            case '\\': value += '\\'; break;
                            case '\'': value += '\''; break;
                            case '"': value += '"'; break;
                            default: value += escaped; break;
                        }
                    } else {
                        value += expr[i];
                    }
                    ++i;
                }
                
                if (i >= expr.length()) {
                    throw JsonPathException("Unterminated string literal", start);
                }
                
                ++i; // Skip closing quote
                tokens.emplace_back(TokenType::STRING, std::move(value), start);
                break;
            }
            
            // Support filter expression tokens
            case '(':
                tokens.emplace_back(TokenType::IDENTIFIER, "(", i);
                ++i;
                break;
            case ')':
                tokens.emplace_back(TokenType::IDENTIFIER, ")", i);
                ++i;
                break;
            case '@':
                tokens.emplace_back(TokenType::IDENTIFIER, "@", i);
                ++i;
                break;
            case '<':
            case '>':
            case '=':
            case '!': {
                // Handle operators like ==, !=, <=, >=
                std::string op(1, c);
                ++i;
                if (i < expr.length() && expr[i] == '=') {
                    op += '=';
                    ++i;
                }
                tokens.emplace_back(TokenType::IDENTIFIER, std::move(op), i - op.length());
                break;
            }
            
            default:
                if (std::isdigit(c) || c == '-') {
                    // Number
                    size_t start = i;
                    std::string value;
                    
                    if (c == '-') {
                        value += c;
                        ++i;
                    }
                    
                    while (i < expr.length() && std::isdigit(expr[i])) {
                        value += expr[i];
                        ++i;
                    }
                    
                    tokens.emplace_back(TokenType::NUMBER, std::move(value), start);
                } else if (isValidIdentifier(c)) {
                    // Identifier
                    size_t start = i;
                    std::string value;
                    
                    while (i < expr.length() && isValidIdentifier(expr[i])) {
                        value += expr[i];
                        ++i;
                    }
                    
                    tokens.emplace_back(TokenType::IDENTIFIER, std::move(value), start);
                } else {
                    throw JsonPathException("Invalid character: " + std::string(1, c), i);
                }
                break;
        }
    }
    
    tokens.emplace_back(TokenType::END, "", expr.length());
    return tokens;
}

void JsonPath::parseExpression(const std::vector<Token>& tokens) {
    if (tokens.empty() || tokens[0].type != TokenType::ROOT) {
        throw JsonPathException("JSONPath expression must start with '$'");
    }
    
    nodes_.emplace_back(NodeType::ROOT);
    size_t pos = 1; // Skip root token
    
    while (pos < tokens.size() && tokens[pos].type != TokenType::END) {
        nodes_.push_back(parseNode(tokens, pos));
    }
    
    // This check ensures that recursive descent '..' is followed by a valid node
    for (size_t i = 0; i < nodes_.size(); ++i) {
        if (nodes_[i].type == NodeType::RECURSIVE) {
            // Recursive descent operator must be followed by another node OR have a target property
            if (i == nodes_.size() - 1 && nodes_[i].property.empty()) {
                throw JsonPathException("Recursive descent operator '..' must be followed by a property or expression");
            }
        }
    }
}

PathNode JsonPath::parseNode(const std::vector<Token>& tokens, size_t& pos) {
    if (pos >= tokens.size()) {
        throw JsonPathException("Unexpected end of expression");
    }
    
    const Token& token = tokens[pos];
    
    switch (token.type) {
        case TokenType::DOT:
            ++pos;
            if (pos >= tokens.size()) {
                throw JsonPathException("Expected property name after '.'", token.position);
            }
            
            if (tokens[pos].type == TokenType::IDENTIFIER) {
                PathNode node(NodeType::PROPERTY, tokens[pos].value);
                ++pos;
                return node;
            } else if (tokens[pos].type == TokenType::WILDCARD) {
                ++pos;
                return PathNode(NodeType::WILDCARD);
            } else {
                throw JsonPathException("Expected property name or '*' after '.'", tokens[pos].position);
            }
            
        case TokenType::BRACKET_OPEN: {
            ++pos;
            if (pos >= tokens.size()) {
                throw JsonPathException("Expected content inside brackets", token.position);
            }
            
            if (tokens[pos].type == TokenType::STRING) {
                // Property access like ['property']
                PathNode node(NodeType::PROPERTY, tokens[pos].value);
                ++pos;
                
                if (pos >= tokens.size() || tokens[pos].type != TokenType::BRACKET_CLOSE) {
                    throw JsonPathException("Expected ']' after string", tokens[pos].position);
                }
                ++pos;
                return node;
            } else if (tokens[pos].type == TokenType::NUMBER) {
                // Index access like [0] or slice like [1:3]
                int first_num = std::stoi(tokens[pos].value);
                ++pos;
                
                if (pos < tokens.size() && tokens[pos].type == TokenType::SLICE) {
                    // Slice notation [start:end]
                    ++pos;
                    int end_num = -1; // Default to end
                    
                    if (pos < tokens.size() && tokens[pos].type == TokenType::NUMBER) {
                        end_num = std::stoi(tokens[pos].value);
                        ++pos;
                    }
                    
                    if (pos >= tokens.size() || tokens[pos].type != TokenType::BRACKET_CLOSE) {
                        throw JsonPathException("Expected ']' after slice", 
                                              pos < tokens.size() ? tokens[pos].position : expression_.length());
                    }
                    
                    ++pos;
                    PathNode node(NodeType::SLICE);
                    node.slice_start = first_num;
                    node.slice_end = end_num;
                    return node;
                } else {
                    // Simple index
                    if (pos >= tokens.size() || tokens[pos].type != TokenType::BRACKET_CLOSE) {
                        throw JsonPathException("Expected ']' after number", 
                                              pos < tokens.size() ? tokens[pos].position : expression_.length());
                    }
                    
                    ++pos;
                    return PathNode(NodeType::INDEX, first_num);
                }
            } else if (tokens[pos].type == TokenType::SLICE) {
                // Slice starting from 0 like [:3]
                ++pos;
                int end_num = -1;
                
                if (pos < tokens.size() && tokens[pos].type == TokenType::NUMBER) {
                    end_num = std::stoi(tokens[pos].value);
                    ++pos;
                }
                
                if (pos >= tokens.size() || tokens[pos].type != TokenType::BRACKET_CLOSE) {
                    throw JsonPathException("Expected ']' after slice", 
                                          pos < tokens.size() ? tokens[pos].position : expression_.length());
                }
                
                ++pos;
                PathNode node(NodeType::SLICE);
                node.slice_start = 0;
                node.slice_end = end_num;
                return node;
            } else if (tokens[pos].type == TokenType::WILDCARD) {
                // Wildcard [*]
                ++pos;
                
                if (pos >= tokens.size() || tokens[pos].type != TokenType::BRACKET_CLOSE) {
                    throw JsonPathException("Expected ']' after '*'", 
                                          pos < tokens.size() ? tokens[pos].position : expression_.length());
                }
                
                ++pos;
                return PathNode(NodeType::WILDCARD);
            } else if (tokens[pos].type == TokenType::FILTER) {
                // Filter expression [?(...)]
                ++pos;
                std::string filter_expr;
                int paren_depth = 0;
                
                // Collect filter expression until closing bracket
                while (pos < tokens.size() && 
                       !(tokens[pos].type == TokenType::BRACKET_CLOSE && paren_depth == 0)) {
                    if (tokens[pos].value == "(") paren_depth++;
                    else if (tokens[pos].value == ")") paren_depth--;
                    
                    filter_expr += tokens[pos].value;
                    ++pos;
                }
                
                if (pos >= tokens.size()) {
                    throw JsonPathException("Expected ']' after filter expression", expression_.length());
                }
                
                ++pos; // Skip closing bracket
                PathNode node(NodeType::FILTER);
                node.filter_expr = std::move(filter_expr);
                return node;
            } else {
                throw JsonPathException("Invalid bracket content", tokens[pos].position);
            }
        }
        
        case TokenType::RECURSIVE:
            ++pos;
            // Check if followed by identifier for recursive property search
            if (pos < tokens.size() && tokens[pos].type == TokenType::IDENTIFIER) {
                PathNode node(NodeType::RECURSIVE);
                node.property = tokens[pos].value;
                ++pos;
                return node;
            }
            return PathNode(NodeType::RECURSIVE);
            
        default:
            throw JsonPathException("Unexpected token: " + token.value, token.position);
    }
}

QueryResult JsonPath::evaluate(const JsonValue& root) const {
    std::vector<std::reference_wrapper<const JsonValue>> current_values;
    std::vector<std::string> current_paths;
    
    // Start with root
    current_values.emplace_back(std::cref(root));
    current_paths.emplace_back("$");
    
    // Apply each node in sequence
    for (const auto& node : nodes_) {
        if (node.type == NodeType::ROOT) {
            continue; // Already handled
        }
        
        std::vector<std::reference_wrapper<const JsonValue>> next_values;
        std::vector<std::string> next_paths;
        
        evaluateNode(node, current_values, current_paths, next_values, next_paths);
        
        current_values = std::move(next_values);
        current_paths = std::move(next_paths);
    }
    
    QueryResult result;
    result.values = std::move(current_values);
    result.paths = std::move(current_paths);
    return result;
}

MutableQueryResult JsonPath::evaluateMutable(JsonValue& root) const {
    std::vector<std::reference_wrapper<JsonValue>> current_values;
    std::vector<std::string> current_paths;
    
    // Start with root
    current_values.emplace_back(std::ref(root));
    current_paths.emplace_back("$");
    
    // Apply each node in sequence
    for (const auto& node : nodes_) {
        if (node.type == NodeType::ROOT) {
            continue; // Already handled
        }
        
        std::vector<std::reference_wrapper<JsonValue>> next_values;
        std::vector<std::string> next_paths;
        
        evaluateNodeMutable(node, current_values, current_paths, next_values, next_paths);
        
        current_values = std::move(next_values);
        current_paths = std::move(next_paths);
    }
    
    MutableQueryResult result;
    result.values = std::move(current_values);
    result.paths = std::move(current_paths);
    return result;
}

void JsonPath::evaluateNode(const PathNode& node,
                           const std::vector<std::reference_wrapper<const JsonValue>>& inputs,
                           const std::vector<std::string>& input_paths,
                           std::vector<std::reference_wrapper<const JsonValue>>& outputs,
                           std::vector<std::string>& output_paths) const {
    switch (node.type) {
        case NodeType::PROPERTY:
            evaluateProperty(node.property, inputs, input_paths, outputs, output_paths);
            break;
            
        case NodeType::INDEX:
            evaluateIndex(node.index, inputs, input_paths, outputs, output_paths);
            break;
            
        case NodeType::SLICE:
            evaluateSlice(node.slice_start, node.slice_end, inputs, input_paths, outputs, output_paths);
            break;
            
        case NodeType::WILDCARD:
            evaluateWildcard(inputs, input_paths, outputs, output_paths);
            break;
            
        case NodeType::RECURSIVE:
            evaluateRecursive(node, inputs, input_paths, outputs, output_paths);
            break;
            
        case NodeType::FILTER:
            evaluateFilter(node.filter_expr, inputs, input_paths, outputs, output_paths);
            break;
            
        default:
            break;
    }
}

void JsonPath::evaluateNodeMutable(const PathNode& node,
                                  const std::vector<std::reference_wrapper<JsonValue>>& inputs,
                                  const std::vector<std::string>& input_paths,
                                  std::vector<std::reference_wrapper<JsonValue>>& outputs,
                                  std::vector<std::string>& output_paths) const {
    switch (node.type) {
        case NodeType::PROPERTY:
            evaluatePropertyMutable(node.property, inputs, input_paths, outputs, output_paths);
            break;
            
        case NodeType::INDEX:
            evaluateIndexMutable(node.index, inputs, input_paths, outputs, output_paths);
            break;
            
        case NodeType::SLICE:
            evaluateSliceMutable(node.slice_start, node.slice_end, inputs, input_paths, outputs, output_paths);
            break;
            
        case NodeType::WILDCARD:
            evaluateWildcardMutable(inputs, input_paths, outputs, output_paths);
            break;
            
        case NodeType::RECURSIVE:
            evaluateRecursiveMutable(node, inputs, input_paths, outputs, output_paths);
            break;
            
        case NodeType::FILTER:
            evaluateFilterMutable(node.filter_expr, inputs, input_paths, outputs, output_paths);
            break;
            
        default:
            break;
    }
}

void JsonPath::evaluateProperty(const std::string& property,
                               const std::vector<std::reference_wrapper<const JsonValue>>& inputs,
                               const std::vector<std::string>& input_paths,
                               std::vector<std::reference_wrapper<const JsonValue>>& outputs,
                               std::vector<std::string>& output_paths) const {
    for (size_t i = 0; i < inputs.size(); ++i) {
        const auto& value = inputs[i].get();
        
        if (value.isObject()) {
            std::string_view prop_view(property);
            if (value.contains(prop_view)) {
                const auto& child = value[prop_view];
                if (!child.isNull()) {  // Only add non-null values
                    outputs.emplace_back(std::cref(child));
                    output_paths.emplace_back(input_paths[i] + "." + property);
                }
            }
        }
    }
}

void JsonPath::evaluatePropertyMutable(const std::string& property,
                                      const std::vector<std::reference_wrapper<JsonValue>>& inputs,
                                      const std::vector<std::string>& input_paths,
                                      std::vector<std::reference_wrapper<JsonValue>>& outputs,
                                      std::vector<std::string>& output_paths) const {
    for (size_t i = 0; i < inputs.size(); ++i) {
        auto& value = inputs[i].get();
        
        if (value.isObject()) {
            std::string_view prop_view(property);
            if (value.contains(prop_view)) {
                auto& child = value[prop_view];
                if (!child.isNull()) {  // Only add non-null values
                    outputs.emplace_back(std::ref(child));
                    output_paths.emplace_back(input_paths[i] + "." + property);
                }
            }
        }
    }
}

void JsonPath::evaluateIndex(int index,
                            const std::vector<std::reference_wrapper<const JsonValue>>& inputs,
                            const std::vector<std::string>& input_paths,
                            std::vector<std::reference_wrapper<const JsonValue>>& outputs,
                            std::vector<std::string>& output_paths) const {
    for (size_t i = 0; i < inputs.size(); ++i) {
        const auto& value = inputs[i].get();
        
        if (value.isArray()) {
            auto* array = value.getArray();
            if (array) {
                int normalized_index = normalizeArrayIndex(index, array->size());
                
                if (normalized_index >= 0 && normalized_index < static_cast<int>(array->size())) {
                    outputs.emplace_back(std::cref((*array)[normalized_index]));
                    output_paths.emplace_back(input_paths[i] + "[" + std::to_string(normalized_index) + "]");
                }
            }
        }
    }
}

void JsonPath::evaluateIndexMutable(int index,
                                   const std::vector<std::reference_wrapper<JsonValue>>& inputs,
                                   const std::vector<std::string>& input_paths,
                                   std::vector<std::reference_wrapper<JsonValue>>& outputs,
                                   std::vector<std::string>& output_paths) const {
    for (size_t i = 0; i < inputs.size(); ++i) {
        auto& value = inputs[i].get();
        
        if (value.isArray()) {
            auto* array = value.getArray();
            if (array) {
                int normalized_index = normalizeArrayIndex(index, array->size());
                
                if (normalized_index >= 0 && normalized_index < static_cast<int>(array->size())) {
                    outputs.emplace_back(std::ref((*array)[normalized_index]));
                    output_paths.emplace_back(input_paths[i] + "[" + std::to_string(normalized_index) + "]");
                }
            }
        }
    }
}

void JsonPath::evaluateSlice(int start, int end,
                            const std::vector<std::reference_wrapper<const JsonValue>>& inputs,
                            const std::vector<std::string>& input_paths,
                            std::vector<std::reference_wrapper<const JsonValue>>& outputs,
                            std::vector<std::string>& output_paths) const {
    for (size_t i = 0; i < inputs.size(); ++i) {
        const auto& value = inputs[i].get();
        
        if (value.isArray()) {
            auto opt_array = value.getArray();
            if (opt_array) {
                const auto& array = *opt_array;
                int array_size = static_cast<int>(array.size());
                
                int norm_start = normalizeArrayIndex(start, array.size());
                int norm_end = (end == -1) ? array_size : normalizeArrayIndex(end, array.size());
                
                norm_start = std::max(0, norm_start);
                norm_end = std::min(array_size, norm_end);
                
                for (int j = norm_start; j < norm_end; ++j) {
                    outputs.emplace_back(std::cref(array[j]));
                    output_paths.emplace_back(input_paths[i] + "[" + std::to_string(j) + "]");
                }
            }
        }
    }
}

void JsonPath::evaluateSliceMutable(int start, int end,
                                   const std::vector<std::reference_wrapper<JsonValue>>& inputs,
                                   const std::vector<std::string>& input_paths,
                                   std::vector<std::reference_wrapper<JsonValue>>& outputs,
                                   std::vector<std::string>& output_paths) const {
    for (size_t i = 0; i < inputs.size(); ++i) {
        auto& value = inputs[i].get();
        
        if (value.isArray()) {
            auto* array = value.getArray();
            if (array) {
                int array_size = static_cast<int>(array->size());
                int norm_start = (start < 0) ? std::max(0, array_size + start) : std::min(start, array_size);
                int norm_end = (end < 0) ? array_size : std::min(end, array_size);
                
                for (int j = norm_start; j < norm_end; ++j) {
                    outputs.emplace_back(std::ref((*array)[j]));
                    output_paths.emplace_back(input_paths[i] + "[" + std::to_string(j) + "]");
                }
            }
        }
    }
}

void JsonPath::evaluateWildcard(const std::vector<std::reference_wrapper<const JsonValue>>& inputs,
                               const std::vector<std::string>& input_paths,
                               std::vector<std::reference_wrapper<const JsonValue>>& outputs,
                               std::vector<std::string>& output_paths) const {
    for (size_t i = 0; i < inputs.size(); ++i) {
        const auto& value = inputs[i].get();
        
        if (value.isObject()) {
            auto opt_object = value.getObject();
            if (opt_object) {
                const auto& object = *opt_object;
                for (const auto& [key, val] : object) {
                    outputs.emplace_back(std::cref(val));
                    output_paths.emplace_back(input_paths[i] + "." + key);
                }
            }
        } else if (value.isArray()) {
            auto opt_array = value.getArray();
            if (opt_array) {
                const auto& array = *opt_array;
                for (size_t j = 0; j < array.size(); ++j) {
                    outputs.emplace_back(std::cref(array[j]));
                    output_paths.emplace_back(input_paths[i] + "[" + std::to_string(j) + "]");
                }
            }
        }
    }
}

void JsonPath::evaluateWildcardMutable(const std::vector<std::reference_wrapper<JsonValue>>& inputs,
                                      const std::vector<std::string>& input_paths,
                                      std::vector<std::reference_wrapper<JsonValue>>& outputs,
                                      std::vector<std::string>& output_paths) const {
    for (size_t i = 0; i < inputs.size(); ++i) {
        auto& value = inputs[i].get();
        
        if (value.isObject()) {
            auto* obj = value.getObject();
            if (obj) {
                for (auto& [key, child] : *obj) {
                    if (!child.isNull()) {
                        outputs.emplace_back(std::ref(child));
                        output_paths.emplace_back(input_paths[i] + "." + std::string(key));
                    }
                }
            }
        } else if (value.isArray()) {
            auto* array = value.getArray();
            if (array) {
                for (size_t j = 0; j < array->size(); ++j) {
                    outputs.emplace_back(std::ref((*array)[j]));
                    output_paths.emplace_back(input_paths[i] + "[" + std::to_string(j) + "]");
                }
            }
        }
    }
}

void JsonPath::evaluateRecursive(const PathNode& node,
                                const std::vector<std::reference_wrapper<const JsonValue>>& inputs,
                                const std::vector<std::string>& input_paths,
                                std::vector<std::reference_wrapper<const JsonValue>>& outputs,
                                std::vector<std::string>& output_paths) const {
    for (size_t i = 0; i < inputs.size(); ++i) {
        if (node.property.empty()) {
            // Standard recursive descent - collect all nodes
            collectRecursive(inputs[i].get(), input_paths[i], outputs, output_paths);
        } else {
            // Recursive search for specific property
            collectRecursiveProperty(inputs[i].get(), input_paths[i], node.property, outputs, output_paths);
        }
    }
}

void JsonPath::collectRecursive(const JsonValue& value, const std::string& base_path,
                               std::vector<std::reference_wrapper<const JsonValue>>& outputs,
                               std::vector<std::string>& output_paths) const {
    // Add current value
    outputs.emplace_back(std::cref(value));
    output_paths.emplace_back(base_path);
    
    // Recursively collect children
    if (value.isObject()) {
        auto opt_object = value.getObject();
        if (opt_object) {
            const auto& object = *opt_object;
            for (const auto& [key, val] : object) {
                collectRecursive(val, base_path + "." + key, outputs, output_paths);
            }
        }
    } else if (value.isArray()) {
        auto opt_array = value.getArray();
        if (opt_array) {
            const auto& array = *opt_array;
            for (size_t j = 0; j < array.size(); ++j) {
                collectRecursive(array[j], base_path + "[" + std::to_string(j) + "]", outputs, output_paths);
            }
        }
    }
}

void JsonPath::collectRecursiveProperty(const JsonValue& value, const std::string& base_path,
                                        const std::string& target_property,
                                        std::vector<std::reference_wrapper<const JsonValue>>& outputs,
                                        std::vector<std::string>& output_paths) const {
    // Check if current value is an object and has the target property
    if (value.isObject()) {
        auto opt_object = value.getObject();
        if (opt_object) {
            const auto& object = *opt_object;
            auto it = object.find(target_property);
            if (it != object.end()) {
                // Found the target property - add its value
                outputs.emplace_back(std::cref(it->second));
                output_paths.emplace_back(base_path + "." + target_property);
            }
            
            // Continue recursive search in all child objects
            for (const auto& [key, val] : object) {
                collectRecursiveProperty(val, base_path + "." + key, target_property, outputs, output_paths);
            }
        }
    } else if (value.isArray()) {
        // Continue recursive search in array elements
        auto opt_array = value.getArray();
        if (opt_array) {
            const auto& array = *opt_array;
            for (size_t j = 0; j < array.size(); ++j) {
                collectRecursiveProperty(array[j], base_path + "[" + std::to_string(j) + "]", target_property, outputs, output_paths);
            }
        }
    }
}

void JsonPath::evaluateFilter(const std::string& filter_expr,
                             const std::vector<std::reference_wrapper<const JsonValue>>& inputs,
                             const std::vector<std::string>& input_paths,
                             std::vector<std::reference_wrapper<const JsonValue>>& outputs,
                             std::vector<std::string>& output_paths) const {
    for (size_t i = 0; i < inputs.size(); ++i) {
        const auto& value = inputs[i].get();
        
        if (value.isArray()) {
            auto opt_array = value.getArray();
            if (opt_array) {
                const auto& array = *opt_array;
                for (size_t j = 0; j < array.size(); ++j) {
                    if (evaluateFilterCondition(filter_expr, array[j])) {
                        outputs.emplace_back(std::cref(array[j]));
                        output_paths.emplace_back(input_paths[i] + "[" + std::to_string(j) + "]");
                    }
                }
            }
        }
    }
}

bool JsonPath::evaluateFilterCondition(const std::string& condition, const JsonValue& context) const {
    // Enhanced filter condition evaluation supporting JSONPath filter expressions
    // Supports patterns like: @.property == value, @['property name'] == value
    // Also supports: @.property != value, @.property <= value, @.property >= value
    
    // Handle whitespace around condition
    std::string trimmed_condition = condition;
    
    // Pattern for bracket notation without quotes (already removed during tokenization): @[property name]
    std::regex bracket_pattern(R"(@\[([^\]]+)\]\s*(==|!=|<=|>=|<|>)\s*([^)]+))");
    std::smatch bracket_match;
    
    // Pattern for dot notation: @.property
    std::regex dot_pattern(R"(@\.(\w+)\s*(==|!=|<=|>=|<|>)\s*([^)]+))");
    std::smatch dot_match;
    
    std::string property;
    std::string op;
    std::string value_str;
    
    // Try bracket notation first
    if (std::regex_search(trimmed_condition, bracket_match, bracket_pattern)) {
        property = bracket_match[1].str();
        op = bracket_match[2].str();
        value_str = bracket_match[3].str();
    }
    // Then try dot notation
    else if (std::regex_search(trimmed_condition, dot_match, dot_pattern)) {
        property = dot_match[1].str();
        op = dot_match[2].str();
        value_str = dot_match[3].str();
    }
    else {
        return false; // No matching pattern found
    }
    
    // Trim whitespace from value_str
    value_str.erase(0, value_str.find_first_not_of(" \t"));
    value_str.erase(value_str.find_last_not_of(" \t") + 1);
    
    // Remove quotes if present
    if (value_str.length() >= 2) {
        if ((value_str.front() == '"' && value_str.back() == '"') ||
            (value_str.front() == '\'' && value_str.back() == '\'')) {
            value_str = value_str.substr(1, value_str.length() - 2);
        }
    }
    
    if (!context.isObject() || !context.contains(property)) {
        return false;
    }
    
    const auto& prop_value = context[property];
    
    // String comparison
    if (prop_value.isString()) {
        auto opt_str = prop_value.getString();
        if (opt_str) {
            std::string prop_str(*opt_str);
            if (op == "==") return prop_str == value_str;
            if (op == "!=") return prop_str != value_str;
            if (op == "<") return prop_str < value_str;
            if (op == ">") return prop_str > value_str;
            if (op == "<=") return prop_str <= value_str;
            if (op == ">=") return prop_str >= value_str;
        }
    }
    // Number comparison
    else if (prop_value.isNumber()) {
        auto opt_num = prop_value.getNumber();
        if (opt_num) {
            try {
                double filter_value = std::stod(value_str);
                double prop_double = *opt_num;
                
                if (op == "==") return std::abs(prop_double - filter_value) < 1e-9;
                if (op == "!=") return std::abs(prop_double - filter_value) >= 1e-9;
                if (op == "<") return prop_double < filter_value;
                if (op == ">") return prop_double > filter_value;
                if (op == "<=") return prop_double <= filter_value;
                if (op == ">=") return prop_double >= filter_value;
            } catch (const std::exception&) {
                return false;
            }
        }
    }
    // Boolean comparison
    else if (prop_value.isBool()) {
        if (value_str == "true" || value_str == "false") {
            bool filter_bool = (value_str == "true");
            bool prop_bool = prop_value.toBool();
            if (op == "==") return prop_bool == filter_bool;
            if (op == "!=") return prop_bool != filter_bool;
        }
    }
    // Null comparison
    else if (prop_value.isNull()) {
        if (value_str == "null") {
            if (op == "==") return true;
            if (op == "!=") return false;
        } else {
            if (op == "==") return false;
            if (op == "!=") return true;
        }
    }
    
    return false;
}

bool JsonPath::exists(const JsonValue& root) const {
    auto result = evaluate(root);
    return !result.empty();
}

std::optional<std::reference_wrapper<const JsonValue>>
JsonPath::selectFirst(const JsonValue& root) const {
    auto result = evaluate(root);
    return result.first();
}

std::vector<std::reference_wrapper<const JsonValue>>
JsonPath::selectAll(const JsonValue& root) const {
    auto result = evaluate(root);
    return result.values;
}

jsonpath::MutableQueryResult queryMutable(JsonValue& root, const std::string& path_expression) {
    jsonpath::JsonPath path(path_expression);
    return path.evaluateMutable(root);
}

std::optional<std::reference_wrapper<JsonValue>> 
JsonPath::selectFirstMutable(JsonValue& root) const {
    auto result = evaluateMutable(root);
    return result.first();
}

std::vector<std::reference_wrapper<JsonValue>>
JsonPath::selectAllMutable(JsonValue& root) const {
    auto result = evaluateMutable(root);
    return std::move(result.values);
}

bool JsonPath::isValidExpression(const std::string& expression) {
    try {
        JsonPath path(expression);
        return true;
    } catch (const JsonPathException&) {
        return false;
    }
}

JsonPath JsonPath::parse(const std::string& expression) {
    return JsonPath(expression);
}

bool JsonPath::isValidIdentifier(char c) {
    return std::isalnum(c) || c == '_' || c == '$';
}

std::string JsonPath::escapeJsonPointerToken(const std::string& token) {
    std::string result;
    for (char c : token) {
        if (c == '~') {
            result += "~0";
        } else if (c == '/') {
            result += "~1";
        } else {
            result += c;
        }
    }
    return result;
}

int JsonPath::normalizeArrayIndex(int index, size_t array_size) {
    if (index < 0) {
        return static_cast<int>(array_size) + index;
    }
    return index;
}

void JsonPath::evaluateRecursiveMutable(const PathNode& node,
                                       const std::vector<std::reference_wrapper<JsonValue>>& inputs,
                                       const std::vector<std::string>& input_paths,
                                       std::vector<std::reference_wrapper<JsonValue>>& outputs,
                                       std::vector<std::string>& output_paths) const {
    // For recursive descent, we need to check if there's a next node
    // If there is, we collect all values and then apply the next operation
    for (size_t i = 0; i < inputs.size(); ++i) {
        auto& value = inputs[i].get();
        const std::string& base_path = input_paths[i];
        
        if (node.property.empty()) {
            // Standard recursive descent - collect all nodes
            collectRecursiveMutable(value, base_path, outputs, output_paths);
        } else {
            // Recursive search for specific property
            collectRecursivePropertyMutable(value, base_path, node.property, outputs, output_paths);
        }
    }
}

void JsonPath::collectRecursiveMutable(JsonValue& value, const std::string& base_path,
                                      std::vector<std::reference_wrapper<JsonValue>>& outputs,
                                      std::vector<std::string>& output_paths) const {
    // Add current value
    outputs.emplace_back(std::ref(value));
    output_paths.emplace_back(base_path);
    
    // Recursively collect from children
    if (value.isObject()) {
        auto* obj = value.getObject();
        if (obj) {
            for (auto& [key, child] : *obj) {
                if (!child.isNull()) {
                    collectRecursiveMutable(child, base_path + "." + std::string(key), outputs, output_paths);
                }
            }
        }
    } else if (value.isArray()) {
        auto* array = value.getArray();
        if (array) {
            for (size_t i = 0; i < array->size(); ++i) {
                collectRecursiveMutable((*array)[i], base_path + "[" + std::to_string(i) + "]", outputs, output_paths);
            }
        }
    }
}

void JsonPath::collectRecursivePropertyMutable(JsonValue& value, const std::string& base_path,
                                              const std::string& target_property,
                                              std::vector<std::reference_wrapper<JsonValue>>& outputs,
                                              std::vector<std::string>& output_paths) const {
    // Check if current value has the target property
    if (value.isObject()) {
        std::string_view prop_view(target_property);
        if (value.contains(prop_view)) {
            auto& child = value[prop_view];
            if (!child.isNull()) {
                outputs.emplace_back(std::ref(child));
                output_paths.emplace_back(base_path + "." + target_property);
            }
        }
        
        // Recursively search in all child values
        auto* obj = value.getObject();
        if (obj) {
            for (auto& [key, child] : *obj) {
                if (!child.isNull()) {
                    collectRecursivePropertyMutable(child, base_path + "." + std::string(key), target_property, outputs, output_paths);
                }
            }
        }
    } else if (value.isArray()) {
        auto* array = value.getArray();
        if (array) {
            for (size_t i = 0; i < array->size(); ++i) {
                collectRecursivePropertyMutable((*array)[i], base_path + "[" + std::to_string(i) + "]", target_property, outputs, output_paths);
            }
        }
    }
}

void JsonPath::evaluateFilterMutable(const std::string& filter_expr,
                                     const std::vector<std::reference_wrapper<JsonValue>>& inputs,
                                     const std::vector<std::string>& input_paths,
                                     std::vector<std::reference_wrapper<JsonValue>>& outputs,
                                     std::vector<std::string>& output_paths) const {
    for (size_t i = 0; i < inputs.size(); ++i) {
        auto& value = inputs[i].get();
        
        if (value.isArray()) {
            auto* array = value.getArray();
            if (array) {
                for (size_t j = 0; j < array->size(); ++j) {
                    auto& item = (*array)[j];
                    if (evaluateFilterCondition(filter_expr, item)) {
                        outputs.emplace_back(std::ref(item));
                        output_paths.emplace_back(input_paths[i] + "[" + std::to_string(j) + "]");
                    }
                }
            }
        }
    }
}

} // namespace jsonpath

namespace jsonvalue_jsonpath {

jsonpath::QueryResult query(const JsonValue& root, const std::string& path_expression) {
    jsonpath::JsonPath path(path_expression);
    return path.evaluate(root);
}

bool exists(const JsonValue& root, const std::string& path_expression) {
    jsonpath::JsonPath path(path_expression);
    return path.exists(root);
}

std::optional<std::reference_wrapper<const JsonValue>>
selectFirst(const JsonValue& root, const std::string& path_expression) {
    jsonpath::JsonPath path(path_expression);
    return path.selectFirst(root);
}

std::vector<std::reference_wrapper<const JsonValue>>
selectAll(const JsonValue& root, const std::string& path_expression) {
    jsonpath::JsonPath path(path_expression);
    return path.selectAll(root);
}

jsonpath::MutableQueryResult queryMutable(JsonValue& root, const std::string& path_expression) {
    jsonpath::JsonPath path(path_expression);
    return path.evaluateMutable(root);
}

std::optional<std::reference_wrapper<JsonValue>>
selectFirstMutable(JsonValue& root, const std::string& path_expression) {
    jsonpath::JsonPath path(path_expression);
    return path.selectFirstMutable(root);
}

std::vector<std::reference_wrapper<JsonValue>>
selectAllMutable(JsonValue& root, const std::string& path_expression) {
    jsonpath::JsonPath path(path_expression);
    return path.selectAllMutable(root);
}

} // namespace jsonvalue_jsonpath
