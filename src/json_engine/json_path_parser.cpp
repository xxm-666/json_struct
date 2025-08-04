#include "json_path_parser.h"
#include "json_path_tokenizer.h"
#include <sstream>
#include <climits>

namespace jsonpath {

std::vector<PathNode> JsonPathParser::parse(const std::vector<Token>& tokens) {
    if (tokens.empty() || tokens[0].type != TokenType::ROOT) {
        throw JsonPathParserException("Expression must start with root '$'", 0);
    }
    
    std::vector<PathNode> nodes;
    nodes.emplace_back(NodeType::ROOT);
    size_t pos = 1; // Skip root token
    
    while (pos < tokens.size() && tokens[pos].type != TokenType::END) {
        PathNode node = parseNode(tokens, pos);
        nodes.push_back(std::move(node));
    }
    
    return nodes;
}

std::vector<PathNode> JsonPathParser::parseUnionExpression(const std::string& expr) {
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
            } else if (c == '[') {
                bracket_depth++;
            } else if (c == ']') {
                bracket_depth--;
            } else if (c == ',' && bracket_depth == 0) {
                if (!current_path.empty()) {
                    // Trim whitespace
                    size_t start = current_path.find_first_not_of(" \t");
                    size_t end = current_path.find_last_not_of(" \t");
                    if (start != std::string::npos) {
                        paths.push_back(current_path.substr(start, end - start + 1));
                    }
                }
                current_path.clear();
                continue;
            }
        } else {
            if (c == string_char && (i == 0 || expr[i-1] != '\\')) {
                in_string = false;
            }
        }
        
        current_path += c;
    }
    
    // Add last path
    if (!current_path.empty()) {
        size_t start = current_path.find_first_not_of(" \t");
        size_t end = current_path.find_last_not_of(" \t");
        if (start != std::string::npos) {
            paths.push_back(current_path.substr(start, end - start + 1));
        }
    }
    
    // Create union node
    std::vector<PathNode> nodes;
    nodes.emplace_back(NodeType::ROOT); // Add ROOT node first
    PathNode union_node(NodeType::UNION);
    union_node.union_paths = std::move(paths);
    nodes.push_back(union_node);
    return nodes;
}

PathNode JsonPathParser::parseNode(const std::vector<Token>& tokens, size_t& pos) {
    if (pos >= tokens.size()) {
        throw JsonPathParserException("Unexpected end of expression", pos);
    }
    
    const Token& token = tokens[pos];
    
    switch (token.type) {
        case TokenType::DOT:
            ++pos;
            if (pos >= tokens.size()) {
                throw JsonPathParserException("Expected property name after '.'", pos);
            }
            
            if (tokens[pos].type == TokenType::IDENTIFIER) {
                std::string property = tokens[pos].value;
                ++pos;
                return PathNode(NodeType::PROPERTY, property);
            } else if (tokens[pos].type == TokenType::WILDCARD) {
                ++pos;
                return PathNode(NodeType::WILDCARD);
            } else {
                throw JsonPathParserException("Expected property name or '*' after '.'", pos);
            }
            
        case TokenType::BRACKET_OPEN: {
            ++pos;
            if (pos >= tokens.size()) {
                throw JsonPathParserException("Expected content inside brackets", pos);
            }
            
            if (tokens[pos].type == TokenType::STRING) {
                // Property access like ['property']
                std::string property = tokens[pos].value;
                ++pos;
                if (pos >= tokens.size() || tokens[pos].type != TokenType::BRACKET_CLOSE) {
                    throw JsonPathParserException("Expected ']' after string", pos);
                }
                ++pos;
                return PathNode(NodeType::PROPERTY, property);
            } else if (tokens[pos].type == TokenType::NUMBER) {
                // Index access like [0] or slice like [1:3] or multiple indices like [0,2,4]
                std::vector<int> indices;
                indices.push_back(std::stoi(tokens[pos].value));
                ++pos;
                
                // Check for additional indices separated by commas
                while (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
                    ++pos; // Skip comma
                    if (pos >= tokens.size() || tokens[pos].type != TokenType::NUMBER) {
                        throw JsonPathParserException("Expected number after comma", pos);
                    }
                    indices.push_back(std::stoi(tokens[pos].value));
                    ++pos;
                }
                
                if (pos < tokens.size() && tokens[pos].type == TokenType::SLICE) {
                    // This is a slice (only if we have a single number)
                    if (indices.size() > 1) {
                        throw JsonPathParserException("Cannot use slice notation with multiple indices", pos);
                    }
                    int start = indices[0];
                    int end = INT_MAX;  // Changed from -1 to INT_MAX for proper slice handling
                    int step = 1;
                    parseSlice(tokens, pos, start, end, step);
                    
                    PathNode node(NodeType::SLICE);
                    node.slice_start = start;
                    node.slice_end = end;
                    node.slice_step = step;
                    return node;
                } else {
                    // Index or multiple indices
                    if (pos >= tokens.size() || tokens[pos].type != TokenType::BRACKET_CLOSE) {
                        throw JsonPathParserException("Expected ']' after number(s)", pos);
                    }
                    ++pos;
                    
                    if (indices.size() == 1) {
                        // Single index
                        return PathNode(NodeType::INDEX, indices[0]);
                    } else {
                        // Multiple indices - create UNION node
                        PathNode node(NodeType::UNION);
                        node.union_indices = std::move(indices);
                        return node;
                    }
                }
            } else if (tokens[pos].type == TokenType::WILDCARD) {
                // Wildcard like [*]
                ++pos;
                if (pos >= tokens.size() || tokens[pos].type != TokenType::BRACKET_CLOSE) {
                    throw JsonPathParserException("Expected ']' after '*'", pos);
                }
                ++pos;
                return PathNode(NodeType::WILDCARD);
            } else if (tokens[pos].type == TokenType::FILTER) {
                // Filter expression like [?(...)]
                ++pos; // Skip '?'
                std::string filter_expr = parseFilterExpression(tokens, pos);
                PathNode node(NodeType::FILTER);
                node.filter_expr = filter_expr;
                return node;
            } else if (tokens[pos].type == TokenType::SLICE) {
                // Slice starting with ':'
                int start = 0;
                int end = INT_MAX;  // Changed from -1 to INT_MAX for proper slice handling
                int step = 1;
                parseSlice(tokens, pos, start, end, step);
                
                PathNode node(NodeType::SLICE);
                node.slice_start = start;
                node.slice_end = end;
                node.slice_step = step;
                return node;
            } else {
                throw JsonPathParserException("Invalid bracket content", pos);
            }
        }
        
        case TokenType::RECURSIVE:
            ++pos;
            // Check if followed by identifier for recursive property search
            if (pos < tokens.size() && tokens[pos].type == TokenType::IDENTIFIER) {
                std::string property = tokens[pos].value;
                ++pos;
                PathNode node(NodeType::RECURSIVE);
                node.property = property;
                return node;
            }
            return PathNode(NodeType::RECURSIVE); // Just recursive descent
            
        default:
            throw JsonPathParserException("Unexpected token: " + token.value, token.position);
    }
}

bool JsonPathParser::hasTopLevelComma(const std::string& expr) {
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

std::string JsonPathParser::parseFilterExpression(const std::vector<Token>& tokens, size_t& pos) {
    std::ostringstream filter_stream;
    int paren_depth = 0;
    int bracket_depth = 0;
    
    // Find the matching closing bracket
    while (pos < tokens.size()) {
        const Token& token = tokens[pos];
        
        // Track nesting depth
        if (token.type == TokenType::PAREN_OPEN) {
            paren_depth++;
        } else if (token.type == TokenType::PAREN_CLOSE) {
            paren_depth--;
        } else if (token.type == TokenType::BRACKET_OPEN) {
            bracket_depth++;
        } else if (token.type == TokenType::BRACKET_CLOSE) {
            if (bracket_depth == 0) {
                // This is our closing bracket
                break;
            }
            bracket_depth--;
        }
        
        // Handle string tokens - add quotes back
        if (token.type == TokenType::STRING) {
            filter_stream << "'" << token.value << "'";
        } else {
            filter_stream << token.value;
        }
        
        // Improved space handling logic
        if (pos + 1 < tokens.size()) {
            const Token& nextToken = tokens[pos + 1];
            bool needSpace = true;
            
            // Cases where we DON'T want spaces:
            if ((token.value == "@" && nextToken.value == ".") ||                              // @.
                (token.value == "." && nextToken.type == TokenType::IDENTIFIER) ||             // .property
                (token.type == TokenType::IDENTIFIER && nextToken.value == ".") ||             // property.
                (token.type == TokenType::IDENTIFIER && nextToken.value == "(") ||             // property(
                (token.type == TokenType::IDENTIFIER && nextToken.type == TokenType::PAREN_OPEN) ||  // property(
                (token.type == TokenType::IDENTIFIER && nextToken.type == TokenType::BRACKET_OPEN) || // property[
                (token.value == "[" && nextToken.value == "?") ||                              // [?
                (token.value == "?" && nextToken.value == "(") ||                              // ?(
                (token.value == ")" && nextToken.value == "]") ||                              // )]
                (nextToken.type == TokenType::BRACKET_CLOSE && bracket_depth == 0) ||          // ...] (our closing bracket)
                (nextToken.type == TokenType::PAREN_CLOSE)) {                                  // ...)
                needSpace = false;
            }
            
            if (needSpace) {
                filter_stream << " ";
            }
        }
        
        ++pos;
    }
    
    if (pos >= tokens.size() || tokens[pos].type != TokenType::BRACKET_CLOSE) {
        throw JsonPathParserException("Expected ']' after filter expression", pos);
    }
    ++pos; // Skip ']'
    
    return filter_stream.str();
}

void JsonPathParser::parseSlice(const std::vector<Token>& tokens, size_t& pos, 
                               int& start, int& end, int& step) {
    // pos should be at ':' token
    if (pos >= tokens.size() || tokens[pos].type != TokenType::SLICE) {
        throw JsonPathParserException("Expected ':' for slice", pos);
    }
    
    ++pos; // Skip ':'
    
    // Parse end
    if (pos < tokens.size() && tokens[pos].type == TokenType::NUMBER) {
        end = std::stoi(tokens[pos].value);
        ++pos;
    }
    
    // Check for step
    if (pos < tokens.size() && tokens[pos].type == TokenType::SLICE) {
        ++pos; // Skip second ':'
        if (pos < tokens.size() && tokens[pos].type == TokenType::NUMBER) {
            step = std::stoi(tokens[pos].value);
            ++pos;
        }
    }
    
    if (pos >= tokens.size() || tokens[pos].type != TokenType::BRACKET_CLOSE) {
        throw JsonPathParserException("Expected ']' after slice", pos);
    }
    ++pos; // Skip ']'
}

} // namespace jsonpath
