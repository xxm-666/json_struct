#pragma once
#include "json_value.h"
#include <functional>
#include <queue>

namespace JsonStruct {

// Streaming JSON parser, supports incremental parsing and large file processing
class JsonStreamParser {
public:
    // Parsing event types
    enum class EventType {
        ObjectStart,     // Object start {
        ObjectEnd,       // Object end }
        ArrayStart,      // Array start [
        ArrayEnd,        // Array end ]
        Key,            // Object key
        Value,          // Value (string, number, boolean, null)
        Error           // Parsing error
    };
    
    // Parsing event
    struct Event {
        EventType type;
        JsonValue value;  // For Key and Value events, contains actual value
        std::string error;        // For Error events, contains error message
        size_t position;          // Position in input
        size_t line;              // Line number
        size_t column;            // Column number
        
        Event(EventType t, size_t pos = 0, size_t l = 1, size_t c = 1) 
            : type(t), position(pos), line(l), column(c) {}
        Event(EventType t, JsonValue val, size_t pos = 0, size_t l = 1, size_t c = 1)
            : type(t), value(std::move(val)), position(pos), line(l), column(c) {}
        Event(const std::string& err, size_t pos = 0, size_t l = 1, size_t c = 1)
            : type(EventType::Error), error(err), position(pos), line(l), column(c) {}
    };
    
    // Event handler type
    using EventHandler = std::function<bool(const Event&)>;
    
private:
    JsonValue::ParseOptions options_;
    EventHandler handler_;
    std::string buffer_;
    size_t position_ = 0;
    size_t line_ = 1;
    size_t column_ = 1;
    std::queue<Event> eventQueue_;
    
public:
    explicit JsonStreamParser(const JsonValue::ParseOptions& options = {})
        : options_(options) {}
    
    // Set event handler
    void setEventHandler(EventHandler handler) {
        handler_ = std::move(handler);
    }
    
    // Add data to parser (supports incremental input)
    void feed(std::string_view data) {
        buffer_.append(data);
        processBuffer();
    }
    
    // Finish parsing (process remaining data)
    void finish() {
        // Process remaining complete JSON structures
        while (position_ < buffer_.size()) {
            if (!processNextToken()) {
                break;
            }
        }
    }
    
    // Reset parser state
    void reset() {
        buffer_.clear();
        position_ = 0;
        line_ = 1;
        column_ = 1;
        while (!eventQueue_.empty()) {
            eventQueue_.pop();
        }
    }
    
    // Get current parsing location info
    std::string getLocationInfo() const {
        return "line " + std::to_string(line_) + ", column " + std::to_string(column_);
    }
    
private:
    void processBuffer() {
        while (position_ < buffer_.size()) {
            if (!processNextToken()) {
                break;  // Need more data
            }
        }
    }
    
    bool processNextToken() {
        skipWhitespace();
        
        if (position_ >= buffer_.size()) {
            return false;  // Need more data
        }
        
        char c = buffer_[position_];
        
        try {
            switch (c) {
                case '{':
                    advance();
                    emitEvent(Event(EventType::ObjectStart, position_, line_, column_));
                    return true;
                    
                case '}':
                    advance();
                    emitEvent(Event(EventType::ObjectEnd, position_, line_, column_));
                    return true;
                    
                case '[':
                    advance();
                    emitEvent(Event(EventType::ArrayStart, position_, line_, column_));
                    return true;
                    
                case ']':
                    advance();
                    emitEvent(Event(EventType::ArrayEnd, position_, line_, column_));
                    return true;
                    
                case '"': {
                    // Parse string (could be key or value)
                    auto value = parseString();
                    if (value.has_value()) {
                        // Check if it's an object key
                        skipWhitespace();
                        if (position_ < buffer_.size() && buffer_[position_] == ':') {
                            emitEvent(Event(EventType::Key, *value, position_, line_, column_));
                        } else {
                            emitEvent(Event(EventType::Value, *value, position_, line_, column_));
                        }
                        return true;
                    }
                    return false;  // Need more data
                }
                
                case 'n': case 't': case 'f': {
                    // Parse null, true, false
                    auto value = parseLiteral();
                    if (value.has_value()) {
                        emitEvent(Event(EventType::Value, *value, position_, line_, column_));
                        return true;
                    }
                    return false;
                }
                
                case '-': case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9': {
                    auto value = parseNumber();
                    if (value.has_value()) {
                        emitEvent(Event(EventType::Value, *value, position_, line_, column_));
                        return true;
                    }
                    return false;
                }
                
                case ',': case ':':
                    advance();  // Skip separator
                    return true;
                    
                default:
                    if (options_.allowRecovery) {
                        advance();  // Skip invalid character
                        return true;
                    } else {
                        emitEvent(Event("Unexpected character '" + std::string(1, c) + "'", 
                                       position_, line_, column_));
                        return false;
                    }
            }
        } catch (const std::exception& e) {
            emitEvent(Event(e.what(), position_, line_, column_));
            return false;
        }
    }
    
    void skipWhitespace() {
        while (position_ < buffer_.size() && std::isspace(buffer_[position_])) {
            advance();
        }
    }
    
    void advance() {
        if (position_ < buffer_.size()) {
            if (buffer_[position_] == '\n') {
                ++line_;
                column_ = 1;
            } else {
                ++column_;
            }
            ++position_;
        }
    }
    
    std::optional<JsonValue> parseString() {
        if (position_ >= buffer_.size() || buffer_[position_] != '"') {
            return std::nullopt;
        }
        
        size_t start = position_;
        advance();  // skip opening quote
        
        std::string result;
        while (position_ < buffer_.size() && buffer_[position_] != '"') {
            if (buffer_[position_] == '\\') {
                advance();
                if (position_ >= buffer_.size()) {
                    return std::nullopt;  // Need more data
                }
                // Simplified escape handling
                char escape = buffer_[position_];
                switch (escape) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case 'n': result += '\n'; break;
                    case 't': result += '\t'; break;
                    case 'r': result += '\r'; break;
                    default: result += escape; break;
                }
            } else {
                result += buffer_[position_];
            }
            advance();
        }
        
        if (position_ >= buffer_.size()) {
            return std::nullopt;  // Need more data
        }
        
        advance();  // skip closing quote
        return JsonValue(std::move(result));
    }
    
    std::optional<JsonValue> parseLiteral() {
        if (buffer_.substr(position_, 4) == "null") {
            position_ += 4;
            column_ += 4;
            return JsonValue();
        } else if (buffer_.substr(position_, 4) == "true") {
            position_ += 4;
            column_ += 4;
            return JsonValue(true);
        } else if (buffer_.substr(position_, 5) == "false") {
            position_ += 5;
            column_ += 5;
            return JsonValue(false);
        }
        
        return std::nullopt;
    }
    
    std::optional<JsonValue> parseNumber() {
        size_t start = position_;
        
        // Simplified number parsing
        if (buffer_[position_] == '-') {
            advance();
        }
        
        while (position_ < buffer_.size() && 
               (std::isdigit(buffer_[position_]) || buffer_[position_] == '.' || 
                buffer_[position_] == 'e' || buffer_[position_] == 'E' ||
                buffer_[position_] == '+' || buffer_[position_] == '-')) {
            advance();
        }
        
        if (position_ == start) {
            return std::nullopt;
        }
        
        auto numStr = buffer_.substr(start, position_ - start);
        
        // Use standard library to parse number
        try {
            if (numStr.find('.') != std::string::npos || 
                numStr.find('e') != std::string::npos || 
                numStr.find('E') != std::string::npos) {
                return JsonValue(std::stod(std::string(numStr)));
            } else {
                return JsonValue(std::stoll(std::string(numStr)));
            }
        } catch (...) {
            return std::nullopt;
        }
    }
    
    void emitEvent(const Event& event) {
        if (handler_) {
            handler_(event);
        } else {
            eventQueue_.push(event);
        }
    }
};

// Simplified streaming JSON builder
class JsonStreamBuilder {
private:
    std::vector<JsonValue> stack_;
    std::vector<std::string> keyStack_;
    JsonValue result_;
    
public:
    void onEvent(const JsonStreamParser::Event& event) {
        switch (event.type) {
            case JsonStreamParser::EventType::ObjectStart:
                startObject();
                break;
                
            case JsonStreamParser::EventType::ObjectEnd:
                endObject();
                break;
                
            case JsonStreamParser::EventType::ArrayStart:
                startArray();
                break;
                
            case JsonStreamParser::EventType::ArrayEnd:
                endArray();
                break;
                
            case JsonStreamParser::EventType::Key:
                setKey(event.value.toString());
                break;
                
            case JsonStreamParser::EventType::Value:
                addValue(event.value);
                break;
                
            case JsonStreamParser::EventType::Error:
                throw std::runtime_error("Parse error: " + event.error);
        }
    }
    
    JsonValue getResult() const {
        return result_;
    }
    
private:
    void startObject() {
        stack_.emplace_back(JsonValue::ObjectType{});
    }
    
    void endObject() {
        if (!stack_.empty()) {
            auto obj = std::move(stack_.back());
            stack_.pop_back();
            if (stack_.empty()) {
                result_ = std::move(obj);
            } else {
                addToParent(std::move(obj));
            }
        }
    }
    
    void startArray() {
        stack_.emplace_back(JsonValue::ArrayType{});
    }
    
    void endArray() {
        if (!stack_.empty()) {
            auto arr = std::move(stack_.back());
            stack_.pop_back();
            if (stack_.empty()) {
                result_ = std::move(arr);
            } else {
                addToParent(std::move(arr));
            }
        }
    }
    
    void setKey(const std::string& key) {
        keyStack_.push_back(key);
    }
    
    void addValue(const JsonValue& value) {
        if (stack_.empty()) {
            result_ = value;
        } else {
            addToParent(value);
        }
    }
    
    void addToParent(const JsonValue& value) {
        if (stack_.empty()) return;
        
        auto& parent = stack_.back();
        if (parent.isObject() && !keyStack_.empty()) {
            parent[keyStack_.back()] = value;
            keyStack_.pop_back();
        } else if (parent.isArray()) {
            parent.append(value);
        }
    }
};

} // namespace JsonStruct
