#include "json_path.h"
#include "json_value.h"
#include <cctype>
#include <sstream>
#include <string_view>
#include <optional>
#include <regex>
#include <iostream>

using JsonValue = JsonStruct::JsonValue;

namespace jsonpath {

inline int normalizeArrayIndex(int index, size_t array_size) {
    if (index < 0) {
        return static_cast<int>(array_size) + index;
    }
    return index;
}

JsonPath::JsonPath(std::string expression) : expression_(std::move(expression)) {
    // Check for union expressions (comma-separated paths at top level)
    if (hasTopLevelComma(expression_)) {
        parseUnionExpression(expression_);
    } else {
        auto tokens = tokenize(expression_);
        parseExpression(tokens);
    }
}

std::vector<Token> JsonPath::tokenize(std::string_view expr) {
    std::vector<Token> tokens;
    size_t i = 0;
	int left_bracket_count = 0;

    while (i < expr.length()) {
        char c = expr[i];
        
        // Skip whitespace
        if (std::isspace(c)) {
            if (left_bracket_count > 0) {
                tokens.emplace_back(TokenType::IDENTIFIER, " ", i);
            }
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
                // tokens.emplace_back(TokenType::FILTER, "?", i);
                // ++i;
                // break;
                {
                // Parse filter expression
                size_t start = i;
                ++i; // Skip '?'

                if (i < expr.length() && expr[i] == '(') {
                    ++i; // Skip '('
                    std::string filter_expr;

                    int paren_depth = 1;
                    while (i < expr.length() && paren_depth > 0) {
                        if (expr[i] == '(') {
                            ++paren_depth;
                        } else if (expr[i] == ')') {
                            --paren_depth;
                        }

                        if (paren_depth > 0) {
                            filter_expr += expr[i];
                        }
                        ++i;
                    }

                    if (paren_depth != 0) {
                        throw JsonPathException("Unterminated filter expression", start);
                    }

                    tokens.emplace_back(TokenType::FILTER, std::move(filter_expr), start);
                } else {
                    throw JsonPathException("Expected '(' after '?'", start);
                }
                break;
            }
                
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
                left_bracket_count++;
                tokens.emplace_back(TokenType::IDENTIFIER, "(", i);
                ++i;
                break;
            case ')':
                left_bracket_count--;
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
                } else if (i < expr.length() && c == '=' && expr[i] == '~') {
                    op += '~';
                    ++i;
                }
                tokens.emplace_back(TokenType::IDENTIFIER, std::move(op), i - op.length());
                break;
            }
            case '&': 
            case '|': {
                // Handle logical operators like &&, ||
                std::string op(1, c);
                ++i;
                if (i < expr.length() && expr[i] == c) {
                    op += c; // e.g., '&&' or '||'
                    ++i;
                }
                tokens.emplace_back(TokenType::IDENTIFIER, std::move(op), i - op.length());
                break;
            }
            case '/': {
                /// Handle /Xxxxx/ for regex patterns
                size_t start = i;
                ++i; // Skip initial '/'
                std::string value = "/";
                while (i < expr.length() && expr[i] != '/') {
                    if (expr[i] == '\\' && i + 1 < expr.length()) {
                        ++i; // Skip escape character
                        if (expr[i] == 'n') {
                            value += '\n';
                        } else if (expr[i] == 't') {
                            value += '\t';
                        } else if (expr[i] == 'r') {
                            value += '\r';
                        } else if (expr[i] == '\\') {
                            value += '\\'; // Keep the escape character
						} else if (expr[i] == 'd' || expr[i] == 'D' || expr[i] == 'w'
                            || expr[i] == 'W' || expr[i] == 's' || expr[i] == 'S'
                            || expr[i] == 'b' || expr[i] == 'B') {
                            value += '\\'; // Keep the escape character
                            value += expr[i];
                        }
                        else {
                            value += expr[i];
                        }
                    } else {
                        value += expr[i];
                    }
                    ++i;
                }
                if (i >= expr.length() || expr[i] != '/') {
                    throw JsonPathException("Unterminated regex pattern", start);
                }
                value += '/'; // Add closing '/'
                ++i; // Skip closing '/'
                tokens.emplace_back(TokenType::IDENTIFIER, std::move(value), start);
                break;
            }
            case 'i': {
                size_t start = i;
                std::string value = "i";
				bool standalone_in = (i > 0 && expr[i - 1] == ' ' && i + 2 < expr.length() && expr[i+2] == ' '); // Check if 'i' is standalone
                if (standalone_in && expr[i + 1] == 'n') {
                  value += 'n';
                  i += 2; // Skip 'in'
                  tokens.emplace_back(TokenType::IDENTIFIER, std::move(value), start);
                  break;
                } else {
                  [[fullthrough]]; 
                }
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
                    bool hasFunction = false; /// Check if this is a function call
                    
                    while (i < expr.length() && isValidIdentifier(expr[i]) || hasFunction) {
                        if (expr[i] == '(') {
                            hasFunction = true; // Start of function call
                        }
                        if (expr[i] == ')' && hasFunction) {
                            hasFunction = false; // End of function call
                        }
                        value += expr[i];
                        ++i;
                    }
                    if (i < expr.size() && expr[i] == '(') {
                      ++i; // Skip '('
                      for (;std::isspace(expr[i]);) ++i;
                      if (expr[i] != ')') {
                        throw JsonPathException("Expected ')' after function arguments", i);
                      }
                      ++i; // Skip ')'
                      value += "()"; // Add function call notation
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

bool JsonPath::hasTopLevelComma(const std::string& expr) {
    int bracket_depth = 0;
    bool in_string = false;
    char string_char = 0;
    
    for (size_t i = 0; i < expr.length(); ++i) {
        char c = expr[i];
        
        if (!in_string) {
            if (c == '"' || c == '\'') {
                in_string = true;
                string_char = c;
            } else if (c == '[') {
                bracket_depth++;
            } else if (c == ']') {
                bracket_depth--;
            } else if (c == ',' && bracket_depth == 0) {
                return true;
            }
        } else {
            if (c == string_char && (i == 0 || expr[i-1] != '\\')) {
                in_string = false;
            }
        }
    }
    return false;
}

void JsonPath::parseUnionExpression(const std::string& expr) {
    // Split expression by top-level commas
    std::vector<std::string> paths;
    std::string current_path;
    int bracket_depth = 0;
    bool in_string = false;
    char string_char = 0;
    
    for (size_t i = 0; i < expr.length(); ++i) {
        char c = expr[i];
        
        if (!in_string) {
            if (c == '"' || c == '\'') {
                in_string = true;
                string_char = c;
                current_path += c;
            } else if (c == '[') {
                bracket_depth++;
                current_path += c;
            } else if (c == ']') {
                bracket_depth--;
                current_path += c;
            } else if (c == ',' && bracket_depth == 0) {
                // Top-level comma - split here
                std::string trimmed = current_path;
                trimmed.erase(0, trimmed.find_first_not_of(" \t"));
                trimmed.erase(trimmed.find_last_not_of(" \t") + 1);
                if (!trimmed.empty()) {
                    paths.push_back(trimmed);
                }
                current_path.clear();
            } else {
                current_path += c;
            }
        } else {
            current_path += c;
            if (c == string_char && (i == 0 || expr[i-1] != '\\')) {
                in_string = false;
            }
        }
    }
    
    // Add last path
    if (!current_path.empty()) {
        std::string trimmed = current_path;
        trimmed.erase(0, trimmed.find_first_not_of(" \t"));
        trimmed.erase(trimmed.find_last_not_of(" \t") + 1);
        if (!trimmed.empty()) {
            paths.push_back(trimmed);
        }
    }
    
    // Create union node
    nodes_.clear();
    PathNode union_node(NodeType::UNION);
    union_node.union_paths = std::move(paths);
    nodes_.push_back(union_node);
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
                // Index access like [0] or slice like [1:3] or union like [0,2,4]
                std::vector<int> indices;
                indices.push_back(std::stoi(tokens[pos].value));
                ++pos;
                
                // Check for comma (union indices) or colon (slice)
                if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
                    // Union indices: [0,2,4]
                    while (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
                        ++pos; // Skip comma
                        if (pos >= tokens.size() || tokens[pos].type != TokenType::NUMBER) {
                            throw JsonPathException("Expected number after comma in union indices", 
                                                  pos < tokens.size() ? tokens[pos].position : expression_.length());
                        }
                        indices.push_back(std::stoi(tokens[pos].value));
                        ++pos;
                    }
                    
                    if (pos >= tokens.size() || tokens[pos].type != TokenType::BRACKET_CLOSE) {
                        throw JsonPathException("Expected ']' after union indices", 
                                              pos < tokens.size() ? tokens[pos].position : expression_.length());
                    }
                    
                    ++pos;
                    PathNode node(NodeType::UNION);
                    node.union_indices = std::move(indices);
                    return node;
                } else if (pos < tokens.size() && tokens[pos].type == TokenType::SLICE) {
                    // Slice notation [start:end] or [start:end:step]
                    ++pos;
                    int end_num = -1; // Default to end
                    int step_num = 1; // Default step
                    
                    if (pos < tokens.size() && tokens[pos].type == TokenType::NUMBER) {
                        end_num = std::stoi(tokens[pos].value);
                        ++pos;
                    }
                    
                    // Check for step
                    if (pos < tokens.size() && tokens[pos].type == TokenType::SLICE) {
                        ++pos;
                        if (pos < tokens.size() && tokens[pos].type == TokenType::NUMBER) {
                            step_num = std::stoi(tokens[pos].value);
                            ++pos;
                        }
                    }
                    
                    if (pos >= tokens.size() || tokens[pos].type != TokenType::BRACKET_CLOSE) {
                        throw JsonPathException("Expected ']' after slice", 
                                              pos < tokens.size() ? tokens[pos].position : expression_.length());
                    }
                    
                    ++pos;
                    PathNode node(NodeType::SLICE);
                    node.slice_start = indices[0];
                    node.slice_end = end_num;
                    node.slice_step = step_num;
                    return node;
                } else {
                    // Simple index
                    if (pos >= tokens.size() || tokens[pos].type != TokenType::BRACKET_CLOSE) {
                        throw JsonPathException("Expected ']' after number", 
                                              pos < tokens.size() ? tokens[pos].position : expression_.length());
                    }
                    
                    ++pos;
                    return PathNode(NodeType::INDEX, indices[0]);
                }
            } else if (tokens[pos].type == TokenType::SLICE) {
                // Slice starting from 0 like [:3] or [:3:2]
                ++pos;
                int end_num = -1;
                int step_num = 1;
                
                if (pos < tokens.size() && tokens[pos].type == TokenType::NUMBER) {
                    end_num = std::stoi(tokens[pos].value);
                    ++pos;
                }
                
                // Check for step
                if (pos < tokens.size() && tokens[pos].type == TokenType::SLICE) {
                    ++pos;
                    if (pos < tokens.size() && tokens[pos].type == TokenType::NUMBER) {
                        step_num = std::stoi(tokens[pos].value);
                        ++pos;
                    }
                }
                
                if (pos >= tokens.size() || tokens[pos].type != TokenType::BRACKET_CLOSE) {
                    throw JsonPathException("Expected ']' after slice", 
                                          pos < tokens.size() ? tokens[pos].position : expression_.length());
                }
                
                ++pos;
                PathNode node(NodeType::SLICE);
                node.slice_start = 0;
                node.slice_end = end_num;
                node.slice_step = step_num;
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
                std::string filter_expr;
				filter_expr = tokens[pos].value; 
                pos += 2; // Skip closing bracket
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
            return PathNode(NodeType::RECURSIVE); // Just recursive descent
        default:
            throw JsonPathException("Unexpected token: " + token.value, token.position);
    }
}

template<typename Ref, typename OutRef, typename QueryResultT, typename F>
QueryResultT evaluateGeneric(JsonValue& root, const std::vector<PathNode>& nodes, F&& evaluate) {
    std::vector<Ref> current_values;
    std::vector<std::string> current_paths;

    // Start with root
    current_values.emplace_back(std::ref(root));
    current_paths.emplace_back("$");

    // Apply each node in sequence
    for (const auto& node : nodes) {
        if (node.type == NodeType::ROOT) {
            continue; // Already handled
        }

        std::vector<OutRef> next_values;
        std::vector<std::string> next_paths;

		evaluate(node, current_values, current_paths, next_values, next_paths);

        current_values = std::move(next_values);
        current_paths = std::move(next_paths);
    }

    QueryResultT result;
    result.values = std::move(current_values);
    result.paths = std::move(current_paths);
    return result;
}

QueryResult JsonPath::evaluate(const JsonValue& root) const {
    return evaluateGeneric<
        std::reference_wrapper<const JsonValue>,
        std::reference_wrapper<const JsonValue>,
        QueryResult>(
            std::remove_const_t<JsonValue&>(root),
            nodes_,
            std::bind(&JsonPath::evaluateNode, this, 
                std::placeholders::_1, 
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4,
                std::placeholders::_5));
}

MutableQueryResult JsonPath::evaluateMutable(JsonValue& root) const {
    return evaluateGeneric<
        std::reference_wrapper<JsonValue>,
        std::reference_wrapper<JsonValue>,
        MutableQueryResult>(
            root,
            nodes_,
            std::bind(&JsonPath::evaluateNodeMutable, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4,
                std::placeholders::_5));
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
            evaluateSlice(node.slice_start, node.slice_end, node.slice_step, inputs, input_paths, outputs, output_paths);
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
            
        case NodeType::UNION:
            evaluateUnion(node, inputs, input_paths, outputs, output_paths);
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
            evaluateSliceMutable(node.slice_start, node.slice_end, node.slice_step, inputs, input_paths, outputs, output_paths);
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
            
        case NodeType::UNION:
            evaluateUnionMutable(node, inputs, input_paths, outputs, output_paths);
            break;
            
        default:
            break;
    }
}

template<typename InType>
void evaluatePropertyGeneric(const std::string& property, 
                      const std::vector<InType>& inputs,
                      const std::vector<std::string>& input_paths,
                      std::vector<InType>& outputs,
                      std::vector<std::string>& output_paths)
{
    for (size_t i = 0; i < inputs.size(); ++i) {
        auto& value = inputs[i].get();

        if (value.isObject()) {
            std::string_view prop_view(property);
            if (value.contains(prop_view)) {
                auto& child = value[prop_view];
                outputs.emplace_back(std::ref(child));
                output_paths.emplace_back(input_paths[i] + "." + property);
            }
        }
    }
}

void JsonPath::evaluateProperty(const std::string& property,
                               const std::vector<std::reference_wrapper<const JsonValue>>& inputs,
                               const std::vector<std::string>& input_paths,
                               std::vector<std::reference_wrapper<const JsonValue>>& outputs,
                               std::vector<std::string>& output_paths) const {

    evaluatePropertyGeneric<std::reference_wrapper<const JsonValue>>(property, inputs, input_paths, outputs, output_paths);
}

void JsonPath::evaluatePropertyMutable(const std::string& property,
                                      const std::vector<std::reference_wrapper<JsonValue>>& inputs,
                                      const std::vector<std::string>& input_paths,
                                      std::vector<std::reference_wrapper<JsonValue>>& outputs,
                                      std::vector<std::string>& output_paths) const {
    evaluatePropertyGeneric<std::reference_wrapper<JsonValue>>(property, inputs, input_paths, outputs, output_paths);
}

template<typename InType>
void evaluateIndexGeneric(int index,
    const std::vector<InType>& inputs,
    const std::vector<std::string>& input_paths,
    std::vector<InType>& outputs,
    std::vector<std::string>& output_paths)
{
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
void JsonPath::evaluateIndex(int index,
                            const std::vector<std::reference_wrapper<const JsonValue>>& inputs,
                            const std::vector<std::string>& input_paths,
                            std::vector<std::reference_wrapper<const JsonValue>>& outputs,
                            std::vector<std::string>& output_paths) const {
    evaluateIndexGeneric<std::reference_wrapper<const JsonValue>>(index, inputs, input_paths, outputs, output_paths);
}

void JsonPath::evaluateIndexMutable(int index,
                                   const std::vector<std::reference_wrapper<JsonValue>>& inputs,
                                   const std::vector<std::string>& input_paths,
                                   std::vector<std::reference_wrapper<JsonValue>>& outputs,
                                   std::vector<std::string>& output_paths) const {
    evaluateIndexGeneric<std::reference_wrapper<JsonValue>>(index, inputs, input_paths, outputs, output_paths);
}

template<typename InRef>
void evaluateSliceGeneric(int start, int end, int step,
    const std::vector<InRef>& inputs,
    const std::vector<std::string>& input_paths,
    std::vector<InRef>& outputs,
    std::vector<std::string>& output_paths)
{
    for (size_t i = 0; i < inputs.size(); ++i) {
        auto& value = inputs[i].get();

        if (value.isArray()) {
            auto opt_array = value.getArray();
            if (opt_array) {
                auto& array = *opt_array;
                int array_size = static_cast<int>(array.size());

                int norm_start = normalizeArrayIndex(start, array.size());
                int norm_end = normalizeArrayIndex(end, array.size());

                norm_start = std::max(0, norm_start);
                norm_end = std::min(array_size, norm_end);

                if (step > 0) {
                    for (int j = norm_start; j < norm_end; j += step) {
                        outputs.emplace_back(std::ref(array[j]));
                        output_paths.emplace_back(input_paths[i] + "[" + std::to_string(j) + "]");
                    }
                }
                else if (step < 0) {
                    for (int j = norm_start; j > norm_end; j += step) {
                        if (j >= 0 && j < array_size) {
                            outputs.emplace_back(std::ref(array[j]));
                            output_paths.emplace_back(input_paths[i] + "[" + std::to_string(j) + "]");
                        }
                    }
                }
            }
        }
    }
}

void JsonPath::evaluateSlice(int start, int end, int step,
                            const std::vector<std::reference_wrapper<const JsonValue>>& inputs,
                            const std::vector<std::string>& input_paths,
                            std::vector<std::reference_wrapper<const JsonValue>>& outputs,
                            std::vector<std::string>& output_paths) const {
    evaluateSliceGeneric<std::reference_wrapper<const JsonValue>>(start, end, step, inputs, input_paths, outputs, output_paths);
}

void JsonPath::evaluateSliceMutable(int start, int end, int step,
                                   const std::vector<std::reference_wrapper<JsonValue>>& inputs,
                                   const std::vector<std::string>& input_paths,
                                   std::vector<std::reference_wrapper<JsonValue>>& outputs,
                                   std::vector<std::string>& output_paths) const {
    evaluateSliceGeneric<std::reference_wrapper<JsonValue>>(start, end, step, inputs, input_paths, outputs, output_paths);
}

template<typename InRef>
void evaluateWildcardGeneric(const std::vector<InRef>& inputs,
                             const std::vector<std::string>& input_paths,
                             std::vector<InRef>& outputs,
                             std::vector<std::string>& output_paths)
{
    for (size_t i = 0; i < inputs.size(); ++i) {
        auto& value = inputs[i].get();
        
        if (value.isObject()) {
            auto opt_object = value.getObject();
            if (opt_object) {
                auto& object = *opt_object;
                for (auto& [key, val] : object) {
                    outputs.emplace_back(std::ref(val));
                    output_paths.emplace_back(input_paths[i] + "." + key);
                }
            }
        } else if (value.isArray()) {
            auto opt_array = value.getArray();
            if (opt_array) {
                auto& array = *opt_array;
                for (size_t j = 0; j < array.size(); ++j) {
                    outputs.emplace_back(std::ref(array[j]));
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
    evaluateWildcardGeneric<std::reference_wrapper<const JsonValue>>(inputs, input_paths, outputs, output_paths);
}

void JsonPath::evaluateWildcardMutable(const std::vector<std::reference_wrapper<JsonValue>>& inputs,
                                      const std::vector<std::string>& input_paths,
                                      std::vector<std::reference_wrapper<JsonValue>>& outputs,
                                      std::vector<std::string>& output_paths) const {
    evaluateWildcardGeneric<std::reference_wrapper<JsonValue>>(inputs, input_paths, outputs, output_paths);
}

template<typename InRef>
void evaluateRecursiveGeneric(const PathNode& node,
    const std::vector<InRef>& inputs,
    const std::vector<std::string>& input_paths,
    std::vector<InRef>& outputs,
    std::vector<std::string>& output_paths)
{
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

template<typename Ref, typename OutRef, typename F>
void evaluateFilterGeneric(const std::string& filter_expr,
    const std::vector<Ref>& inputs,
    const std::vector<std::string>& input_paths,
    std::vector<OutRef>& outputs,
    std::vector<std::string>& output_paths,
    F&& filter)
{
    auto has_nested_filter = filter_expr.find('?') != std::string::npos;
    if (!has_nested_filter) {
        for (size_t i = 0; i < inputs.size(); ++i) {
            auto& value = inputs[i].get();
            if (auto opt_array = value.toArray()) {
                auto& array = opt_array->get();
                for (size_t j = 0; j < array.size(); ++j) {
                    if (filter(filter_expr, array[j])) {
                        outputs.emplace_back(array[j]);
                        output_paths.emplace_back(input_paths[i] + "[" + std::to_string(j) + "]");
                    }
                }
            }
            else if (value.isObject()) {
                if (filter(filter_expr, value)) {
                    outputs.emplace_back(inputs[i]);
                    output_paths.emplace_back(input_paths[i]);
                }
            }
        }
        return;
    }

    for (size_t i = 0; i < inputs.size(); ++i) {
        auto& value = inputs[i].get();
        if (auto opt_array = value.toArray()) {
            auto& array = opt_array->get();
            for (size_t j = 0; j < array.size(); ++j) {
                if (filter(filter_expr, array[j])) {
                    outputs.emplace_back(array[j]);
                    output_paths.emplace_back(input_paths[i] + "[" + std::to_string(j) + "]");
                }
            }
        }
        else if (value.isObject()) {
            if (filter(filter_expr, value)) {
                outputs.emplace_back(inputs[i]);
                output_paths.emplace_back(input_paths[i]);
            }
        }
    }
}

void JsonPath::evaluateFilter(const std::string& filter_expr,
    const std::vector<std::reference_wrapper<const JsonValue>>& inputs,
    const std::vector<std::string>& input_paths,
    std::vector<std::reference_wrapper<const JsonValue>>& outputs,
    std::vector<std::string>& output_paths) const 
{
    evaluateFilterGeneric(
            filter_expr, 
            inputs, 
            input_paths, 
            outputs, 
            output_paths, 
            std::bind(&JsonPath::evaluateFilterCondition, 
                this, std::placeholders::_1, 
                std::placeholders::_2));
}

static void tryTrimParen(std::string& expr)
{
    while (expr.length() > 2 &&
        expr.front() == '(' &&
        expr.back() == ')') {
        int paren_count = 0;
        bool matches = true;

        for (size_t i = 0; i < expr.length(); ++i) {
            if (expr[i] == '(') {
                paren_count++;
            }
            else if (expr[i] == ')') {
                paren_count--;
                if (paren_count == 0 && i < expr.length() - 1) {
					matches = false; // front and back parentheses not match
                    break;
                }
            }
        }

        if (matches && paren_count == 0) {
            expr = expr.substr(1, expr.length() - 2);
        }
        else {
            break;
        }
    }
}

std::tuple<size_t, size_t> findToplevelLogical(const std::string& expr)
{
    int bracket_depth = 0;
    int paren_depth = 0;
    size_t and_pos = std::string::npos,
        or_pos = std::string::npos;

    for (size_t i = 0; i < expr.length() - 1; ++i) {
        char c = expr[i];
        if (c == '[') bracket_depth++;
        else if (c == ']') bracket_depth--;
        else if (c == '(') paren_depth++;
        else if (c == ')') paren_depth--;
        else if (bracket_depth == 0 && paren_depth == 0) {
            if (c == '&' && expr[i + 1] == '&' && and_pos == std::string::npos) {
                and_pos = i;
            }
            else if (c == '|' && expr[i + 1] == '|' && or_pos == std::string::npos) {
                or_pos = i;
            }
        }
    }

    return { or_pos, and_pos };
}

inline auto trimHelper = [](std::string& str) {
    str.erase(0, str.find_first_not_of(" \t"));
    str.erase(str.find_last_not_of(" \t") + 1);
};

bool JsonPath::evaluateFilterCondition(const std::string& condition, const JsonValue& context) const {   
    std::string trimmed_condition = condition;
	/// Remove leading and trailing parentheses if they match
    tryTrimParen(trimmed_condition);
    
    auto [or_pos, and_pos] = findToplevelLogical(trimmed_condition);

	/// the priority of logical operators is '&&' > '||'
    if (or_pos != std::string::npos) {
        std::string left = trimmed_condition.substr(0, or_pos);
        std::string right = trimmed_condition.substr(or_pos + 2);
		trimHelper(left); trimHelper(right);
        return evaluateFilterCondition(left, context) || evaluateFilterCondition(right, context);
    }

    if (and_pos != std::string::npos) {
        std::string left = trimmed_condition.substr(0, and_pos);
        std::string right = trimmed_condition.substr(and_pos + 2);
		trimHelper(left); trimHelper(right);
        return evaluateFilterCondition(left, context) && evaluateFilterCondition(right, context);
    }
    
    auto nested_filter_result = parseNestedFilterExpression(trimmed_condition, context);
    if (nested_filter_result.has_value()) {
        return nested_filter_result.value();
    }
    
    if(filterKey_RegexRegex(trimmed_condition, context)) {
        return true;
	}
    if(filterKey_RegexIn(trimmed_condition, context)) {
        return true;
	}

    return evaluateBasicFilterCondition(trimmed_condition, context);
}

static bool extractProperty(const std::string& condition, std::string& property, std::string& sub_condition)
{
    size_t at_pos = condition.find('@');
    if (at_pos == std::string::npos) {
        return false;
    }

    size_t dot_pos = condition.find('.', at_pos);
    if (dot_pos == std::string::npos) {
        return false;
    }

    size_t bracket_filter_pos = condition.find("[?", dot_pos);
    if (bracket_filter_pos == std::string::npos) {
        return false;
    }

    auto [or_pos, and_pos] = findToplevelLogical(condition);
    if (or_pos != std::string::npos || and_pos != std::string::npos) {
        return false;
    }

    size_t prop_start = dot_pos + 1;
    size_t bracket_pos = condition.find('[', prop_start);
    if (bracket_pos == std::string::npos) {
        return false;
    }

    // Check if this is a filter expression [?(...)]
    if (bracket_pos + 1 >= condition.length() || condition[bracket_pos + 1] != '?') {
        return false;
    }

    // Find the opening parenthesis after ?
    size_t paren_start = bracket_pos + 2;
    if (paren_start >= condition.length() || condition[paren_start] != '(') {
        return false;
    }

    // Find the matching closing parenthesis, 
    // accounting for nested parentheses and brackets
    int paren_depth = 0;
    int bracket_depth = 0;
    size_t paren_end = paren_start;

    for (size_t i = paren_start; i < condition.length(); ++i) {
        char c = condition[i];
        if (c == '(') {
            paren_depth++;
        }
        else if (c == ')') {
            paren_depth--;
            if (paren_depth == 0 && bracket_depth == 0) {
                paren_end = i;
                break;
            }
        }
        else if (c == '[') {
            bracket_depth++;
        }
        else if (c == ']') {
            bracket_depth--;
        }
    }

    if (paren_depth != 0) {
        return false;
    }

    // Find the closing bracket
    size_t bracket_end = condition.find(']', paren_end);
    if (bracket_end == std::string::npos) {
        return false;
    }

    property = condition.substr(prop_start, bracket_pos - prop_start);
    sub_condition = condition.substr(paren_start + 1, paren_end - paren_start - 1);

    return true;
}

/// process nested filter expressions like @.property[?(...)]
std::optional<bool> JsonPath::parseNestedFilterExpression(const std::string& condition, const JsonValue& context) const {
    std::string property, sub_condition;
    if (!extractProperty(condition, property, sub_condition))
        return std::nullopt;
    
    // Check if context has the property and it's an array
    if (!context.isObject() || !context.contains(property)) {
        return false;
    }
    
    const auto& prop_value = context[property];
    if (const auto& opt_array = prop_value.toArray()) {
        const auto& array = opt_array->get();
        // Check if any element in the array satisfies the sub condition
        for (const auto& item : array) {
            if (evaluateFilterCondition(sub_condition, item)) {
                return true;
            }
        }
    }
    
    return false;
}

bool JsonPath::evaluateBasicFilterCondition(const std::string& condition, const JsonValue& context) const {
    
    // Check for field existence pattern: @.property or @['property']
    std::regex existence_dot_pattern(R"(^@\.(\w+)$)");
    std::regex existence_bracket_pattern(R"(^@\[([^\]]+)\]$)");
    std::smatch existence_match;
    
    // Check for field existence (no operator)
    if (std::regex_search(condition, existence_match, existence_dot_pattern)) {
        std::string property = existence_match[1].str();
        return context.isObject() && context.contains(property);
    }
    else if (std::regex_search(condition, existence_match, existence_bracket_pattern)) {
        std::string property = existence_match[1].str();
        return context.isObject() && context.contains(property);
    }
    
    // Pattern for bracket notation without quotes (already removed during tokenization): @[property name]
    std::regex bracket_pattern(R"(@\[\s*(['"]?)([^\]'"]+)\1\s*\]\s*(==|!=|<=|>=|<|>)\s*([^)]+))");
    std::smatch bracket_match;
    
	// Pattern for dot notation: @.property |@.property1.property2(...) |@.property1.property2.(...).method() op value
	// Just supportsingle method call in dot notation
	// TODO: support multiple method calls in dot notation
    std::regex dot_pattern(R"(@\s*((?:\.\w+(?:\(\))?)*)\s*(?:(==|!=|<=|>=|=|<|>)\s*(.*)?)?)");
    std::smatch dot_match;
    
    std::string property, property_string;
    std::string method;
    std::string op;
    std::string value_str;
    std::vector<std::string> property_parts;

    // Try bracket notation first
    if (std::regex_search(condition, bracket_match, bracket_pattern)) {
        property = bracket_match[2].str();
        op = bracket_match[3].str();
        value_str = bracket_match[4].str();
    }
    // Then try dot notation
    else if (std::regex_search(condition, dot_match, dot_pattern)) {
        if (dot_match.size() == 4) {
            property_string = dot_match[1].str();
            op = dot_match[2].str();
            value_str = dot_match[3].str();
        }
        else if(dot_match.size() == 3) {
            property_string = dot_match[1].str();
            op = dot_match[2].str();
		}
        else if (dot_match.size() == 2) {
            property_string = dot_match[1].str();
        }
        else {
            return false; 
        }
    }
    else {
        return false;
    }

    auto hasMethod = property_string.find("()");
    if (hasMethod != std::string::npos) {
        auto lastDot = property_string.find_last_of('.');
        if (lastDot != std::string::npos) {
            method = property_string.substr(lastDot + 1, hasMethod - lastDot - 1);
            property_string = property_string.substr(0, lastDot);
        }
    }
    auto dotPos = property_string.find_first_of('.');
    while (dotPos != std::string::npos) {
        property_string = property_string.substr(dotPos + 1);
        dotPos = property_string.find_first_of('.');
        if (dotPos != std::string::npos) {
            property_parts.push_back(property_string.substr(0, dotPos));
        }
        else
            property_parts.push_back(property_string);
    }
    
    trimHelper(value_str);
    
    // Remove quotes if present
    if (value_str.length() >= 2) {
        if ((value_str.front() == '"' && value_str.back() == '"') ||
            (value_str.front() == '\'' && value_str.back() == '\'')) {
            value_str = value_str.substr(1, value_str.length() - 2);
        }
    }
    
	//Just process value is Number or String
    if (property_parts.empty() && property.empty()) {
        return filterKey_RegexSingleValue(op, value_str, context);
    }

    JsonStruct::JsonValue current_context = context;
    if (!property_parts.empty()) {
        if (property_parts.size() > 1) {
            property = property_parts.back();
            property_parts.pop_back();

            for (const auto& part : property_parts) {
                if (current_context.isObject() && current_context.contains(part)) {
                    current_context = current_context[part];
                }
                else {
                    return false;
                }
            }
        }
        else {
            property = property_parts[0];
        }
    }

    if (current_context.isObject() && current_context.contains(property))
        return filterKey_ValueCalculate(method, op, value_str, current_context[property]);
	
    return filterKey_ValueCalculate(method, op, value_str, current_context);
}

bool JsonPath::filterKey_RegexIn(const std::string& condition, const JsonStruct::JsonValue& context) const
{
	// Pattern for 'value in @.property' or 'value in @['property']'
    std::regex contains_pattern(R"('([^']+)'\s+in\s+@\.(\w+))");
    std::smatch contains_match;
	auto trimmed_condition = condition;
    if (std::regex_search(trimmed_condition, contains_match, contains_pattern)) {
        std::string search_value = contains_match[1].str();
        std::string property = contains_match[2].str();

        if (!context.isObject() || !context.contains(property)) {
            return false;
        }

        const auto& prop_value = context[property];
        if (const auto& opt_array = prop_value.toArray()) {
			const auto& array = opt_array->get();
            for (const auto& item : array) {
                if (item.isString()) {
                    auto opt_str = item.getString();
                    if (opt_str && *opt_str == search_value) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool JsonPath::filterKey_RegexRegex(const std::string& condition, const JsonStruct::JsonValue& context) const
{
    std::regex regex_pattern(R"(@\.(\w+)\s*=~\s*/([^/]+)/)");
    std::smatch regex_match;
	auto trimmed_condition = condition;
    if (std::regex_search(trimmed_condition, regex_match, regex_pattern)) {
        std::string property = regex_match[1].str();
        std::string pattern = regex_match[2].str();

        if (!context.isObject() || !context.contains(property)) {
            return false;
        }

        const auto& prop_value = context[property];
        if (prop_value.isString()) {
            auto opt_str = prop_value.getString();
            if (opt_str) {
                try {
                    std::regex re(pattern);
                    std::string str_value(*opt_str);
                    return std::regex_search(str_value, re);
                }
                catch (const std::exception&) {
                    return false;
                }
            }
        }
    }

    return false;
}

bool JsonPath::filterKey_ValueCalculate(const std::string& method, const std::string& op, const std::string& value_str, const JsonStruct::JsonValue& prop_value) const
{
    if (prop_value.isString()) {
        auto prop_str = prop_value.toString();
        if (method == "length") {
            size_t str_length = prop_str.length();
            try {
                size_t filter_length = std::stoul(value_str);
                if (op == "==") return str_length == filter_length;
                if (op == "!=") return str_length != filter_length;
                if (op == "<") return str_length < filter_length;
                if (op == ">") return str_length > filter_length;
                if (op == "<=") return str_length <= filter_length;
                if (op == ">=") return str_length >= filter_length;
            }
            catch (const std::exception&) {
                return false;
            }
        }
        else if (method == "sum") {
            return prop_str.length();
        }
        else {
            if (op.empty()) return true; // No operator means just check existence
            if (op == "==") return prop_str == value_str;
            if (op == "!=") return prop_str != value_str;
            if (op == "<") return prop_str < value_str;
            if (op == ">") return prop_str > value_str;
            if (op == "<=") return prop_str <= value_str;
            if (op == ">=") return prop_str >= value_str;
        }
    }

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
            }
            catch (const std::exception&) {
                return false;
            }
        }
    }
    else if (prop_value.isBool()) {
        if (value_str == "true" || value_str == "false") {
            bool filter_bool = (value_str == "true");
            bool prop_bool = prop_value.toBool();
            if (op == "==") return prop_bool == filter_bool;
            if (op == "!=") return prop_bool != filter_bool;
        }
    }
    else if (prop_value.isNull()) {
        if (value_str == "null") {
            if (op == "==") return true;
            if (op == "!=") return false;
        }
        else {
            if (op == "==") return false;
            if (op == "!=") return true;
        }
    }
    else if (const auto& opt_array = prop_value.toArray()) {
        if (method.empty())
            return false;
        const auto& array = opt_array->get();

        if (method == "length") {
            size_t array_length = array.size();
            try {
                size_t filter_length = std::stoul(value_str);
                if (op == "==") return array_length == filter_length;
                if (op == "!=") return array_length != filter_length;
                if (op == "<") return array_length < filter_length;
                if (op == ">") return array_length > filter_length;
                if (op == "<=") return array_length <= filter_length;
                if (op == ">=") return array_length >= filter_length;
            }
            catch (const std::exception&) {
                return false;
            }
        }
        else if (method == "max") {
            if (array.empty()) return false;
            double max_value = std::numeric_limits<double>::lowest();
            for (const auto& item : array) {
                if (item.isNumber()) {
                    auto opt_num = item.getNumber();
                    if (opt_num && *opt_num > max_value) {
                        max_value = *opt_num;
                    }
                }
            }
            try {
                double filter_value = std::stod(value_str);
                if (op == "==") return std::abs(max_value - filter_value) < 1e-9;
                if (op == "!=") return std::abs(max_value - filter_value) >= 1e-9;
                if (op == "<") return max_value < filter_value;
                if (op == ">") return max_value > filter_value;
                if (op == "<=") return max_value <= filter_value;
                if (op == ">=") return max_value >= filter_value;
            }
            catch (const std::exception&) {
                return false;
            }
        }
    }

    return false;
}

bool JsonPath::filterKey_RegexSingleValue(const std::string& op, const std::string& opValue, const JsonStruct::JsonValue& value) const
{
    if (value.isString()) {
		auto string_value = value.toString();
        if (op == "==") 
			return string_value == opValue;
		if (op == "!=")
			return string_value != opValue;
    }
    if (value.isNumber()) {
        auto opt_num = value.getNumber();
        if (opt_num) {
            try {
                double num_value = *opt_num;
                double filter_value = std::stod(opValue);
                if (op == "==") return std::abs(num_value - filter_value) < 1e-9;
                if (op == "!=") return std::abs(num_value - filter_value) >= 1e-9;
                if (op == "<") return num_value < filter_value;
                if (op == ">") return num_value > filter_value;
                if (op == "<=") return num_value <= filter_value;
                if (op == ">=") return num_value >= filter_value;
            }
            catch (const std::exception&) {
                return false;
            }
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
    std::vector<std::string>& output_paths) const 
{
    evaluateFilterGeneric(
            filter_expr,
            inputs,
            input_paths,
            outputs,
            output_paths,
            std::bind(&JsonPath::evaluateFilterCondition, 
                this, std::placeholders::_1, 
                std::placeholders::_2));
}

void JsonPath::evaluateUnionMutable(const PathNode& node,
    const std::vector<std::reference_wrapper<JsonValue>>& inputs,
    const std::vector<std::string>& input_paths,
    std::vector<std::reference_wrapper<JsonValue>>& outputs,
    std::vector<std::string>& output_paths) const {
    if (!node.union_indices.empty()) {
        // Union of array indices like [0,2,4]
        for (size_t i = 0; i < inputs.size(); ++i) {
            auto& value = inputs[i].get();

            if (value.isArray()) {
                auto* array = value.getArray();
                if (array) {
                    for (int index : node.union_indices) {
                        int normalized_index = normalizeArrayIndex(index, array->size());
                        if (normalized_index >= 0 && normalized_index < static_cast<int>(array->size())) {
                            outputs.emplace_back(std::ref((*array)[normalized_index]));
                            output_paths.emplace_back(input_paths[i] + "[" + std::to_string(normalized_index) + "]");
                        }
                    }
                }
            }
        }
    }
    else if (!node.union_paths.empty()) {
        // Union of path expressions like $.data.primary.value,$.data.secondary.value
        for (const auto& path_expr : node.union_paths) {
            JsonPath sub_path(path_expr);
            for (size_t i = 0; i < inputs.size(); ++i) {
                auto sub_result = sub_path.evaluateMutable(inputs[i].get());
                for (size_t j = 0; j < sub_result.values.size(); ++j) {
                    outputs.emplace_back(sub_result.values[j]);
                    output_paths.emplace_back(sub_result.paths[j]);
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

// 懒加载支持方法的实现
namespace jsonpath {

bool JsonPath::evaluateSingleNode(const PathNode& node,
                                 const JsonStruct::JsonValue* input, 
                                 const std::string& input_path,
                                 std::vector<std::pair<const JsonStruct::JsonValue*, std::string>>& outputs) const {
    // 将单个输入转换为向量格式以复用现有方法
    std::vector<std::reference_wrapper<const JsonStruct::JsonValue>> inputs;
    std::vector<std::string> input_paths;
    inputs.emplace_back(std::cref(*input));
    input_paths.emplace_back(input_path);
    
    // 输出向量
    std::vector<std::reference_wrapper<const JsonStruct::JsonValue>> raw_outputs;
    std::vector<std::string> output_paths;
    
    // 调用现有的节点评估方法
    evaluateNode(node, inputs, input_paths, raw_outputs, output_paths);
    
    // 转换输出格式
    outputs.clear();
    for (size_t i = 0; i < raw_outputs.size(); ++i) {
        outputs.emplace_back(&raw_outputs[i].get(), output_paths[i]);
    }
    
    return !outputs.empty();
}

void JsonPath::evaluateUnion(const PathNode& node,
                            const std::vector<std::reference_wrapper<const JsonValue>>& inputs,
                            const std::vector<std::string>& input_paths,
                            std::vector<std::reference_wrapper<const JsonValue>>& outputs,
                            std::vector<std::string>& output_paths) const {
    if (!node.union_indices.empty()) {
        // Union of array indices like [0,2,4]
        for (size_t i = 0; i < inputs.size(); ++i) {
            const auto& value = inputs[i].get();
            
            if (value.isArray()) {
                auto opt_array = value.getArray();
                if (opt_array) {
                    const auto& array = *opt_array;
                    for (int index : node.union_indices) {
                        int normalized_index = normalizeArrayIndex(index, array.size());
                        if (normalized_index >= 0 && normalized_index < static_cast<int>(array.size())) {
                            outputs.emplace_back(std::cref(array[normalized_index]));
                            output_paths.emplace_back(input_paths[i] + "[" + std::to_string(normalized_index) + "]");
                        }
                    }
                }
            }
        }
    } else if (!node.union_paths.empty()) {
        // Union of path expressions like $.data.primary.value,$.data.secondary.value
        for (const auto& path_expr : node.union_paths) {
            JsonPath sub_path(path_expr);
            for (size_t i = 0; i < inputs.size(); ++i) {
                auto sub_result = sub_path.evaluate(inputs[i].get());
                for (size_t j = 0; j < sub_result.values.size(); ++j) {
                    outputs.emplace_back(sub_result.values[j]);
                    output_paths.emplace_back(sub_result.paths[j]);
                }
            }
        }
    }
}

} // namespace jsonpath
