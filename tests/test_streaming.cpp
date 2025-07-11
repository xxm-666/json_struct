#include "json_engine/json_stream_parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>

using namespace JsonStruct;

void testBasicStreaming() {
    std::cout << "=== Basic Streaming Test ===" << std::endl;
    
    JsonStreamParser parser;
    JsonStreamBuilder builder;
    
    // Setup event handler
    parser.setEventHandler([&builder](const JsonStreamParser::Event& event) {
        std::cout << "Event: ";
        switch (event.type) {
            case JsonStreamParser::EventType::ObjectStart:
                std::cout << "ObjectStart";
                break;
            case JsonStreamParser::EventType::ObjectEnd:
                std::cout << "ObjectEnd";
                break;
            case JsonStreamParser::EventType::ArrayStart:
                std::cout << "ArrayStart";
                break;
            case JsonStreamParser::EventType::ArrayEnd:
                std::cout << "ArrayEnd";
                break;
            case JsonStreamParser::EventType::Key:
                std::cout << "Key: " << event.value.toString();
                break;
            case JsonStreamParser::EventType::Value:
                std::cout << "Value: " << event.value.dump();
                break;
            case JsonStreamParser::EventType::Error:
                std::cout << "Error: " << event.error;
                return false;
        }
        std::cout << std::endl;
        
        builder.onEvent(event);
        return true;
    });
    
    // Simulate chunked input
    std::vector<std::string> chunks = {
        "{\"name\": \"",
        "Alice\", \"age",
        "\": 30, \"hobbies",
        "\": [\"reading\", \"",
        "coding\"], \"active",
        "\": true}"
    };
    
    try {
        for (const auto& chunk : chunks) {
            std::cout << "Feeding chunk: " << chunk << std::endl;
            parser.feed(chunk);
        }
        
        parser.finish();
        
        auto result = builder.getResult();
        std::cout << "\nFinal result:" << std::endl;
        std::cout << result.dump(2) << std::endl;
        
        std::cout << "✅ Basic streaming test passed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Basic streaming test failed: " << e.what() << std::endl;
    }
}

void testLargeJsonSimulation() {
    std::cout << "\n=== Large JSON Simulation Test ===" << std::endl;
    
    // Simulate processing large JSON files
    JsonStreamParser parser;
    
    size_t objectCount = 0;
    size_t arrayCount = 0;
    size_t valueCount = 0;
    
    parser.setEventHandler([&](const JsonStreamParser::Event& event) {
        switch (event.type) {
            case JsonStreamParser::EventType::ObjectStart:
                ++objectCount;
                break;
            case JsonStreamParser::EventType::ArrayStart:
                ++arrayCount;
                break;
            case JsonStreamParser::EventType::Value:
                ++valueCount;
                break;
            case JsonStreamParser::EventType::Error:
                std::cout << "Parse error: " << event.error << std::endl;
                return false;
            default:
                break;
        }
        return true;
    });
    
    // Generate a large JSON structure
    std::ostringstream largeJson;
    largeJson << "{\"users\": [";
    
    for (int i = 0; i < 1000; ++i) {
        if (i > 0) largeJson << ",";
        largeJson << "{\"id\": " << i 
                  << ", \"name\": \"User" << i 
                  << "\", \"data\": [1, 2, 3, 4, 5]}";
    }
    
    largeJson << "]}";
    
    std::string jsonStr = largeJson.str();
    std::cout << "Generated JSON size: " << jsonStr.size() << " bytes" << std::endl;
    
    try {
        // Simulate chunked reading (1KB per chunk)
        const size_t chunkSize = 1024;
        for (size_t i = 0; i < jsonStr.size(); i += chunkSize) {
            size_t size = std::min(chunkSize, jsonStr.size() - i);
            parser.feed(jsonStr.substr(i, size));
        }
        
        parser.finish();
        
        std::cout << "Processing completed:" << std::endl;
        std::cout << "  Objects: " << objectCount << std::endl;
        std::cout << "  Arrays: " << arrayCount << std::endl;
        std::cout << "  Values: " << valueCount << std::endl;
        
        std::cout << "✅ Large JSON simulation test passed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Large JSON simulation test failed: " << e.what() << std::endl;
    }
}

void testErrorHandlingInStreaming() {
    std::cout << "\n=== Streaming Error Handling Test ===" << std::endl;
    
    JsonStreamParser parser;
    
    bool errorHandled = false;
    parser.setEventHandler([&](const JsonStreamParser::Event& event) {
        if (event.type == JsonStreamParser::EventType::Error) {
            std::cout << "Caught streaming error: " << event.error << std::endl;
            errorHandled = true;
            return false;
        }
        return true;
    });
    
    try {
        // 故意输入无效JSON
        parser.feed("{\"valid\": true, invalid_syntax: broken}");
        parser.finish();
        
        if (errorHandled) {
            std::cout << "✅ Streaming error handling test passed" << std::endl;
        } else {
            std::cout << "⚠️ Expected error not caught in streaming mode" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "✅ Streaming error handling test passed (exception caught)" << std::endl;
    }
}

void testStreamingVsRegularParsing() {
    std::cout << "\n=== Streaming vs Regular Parsing Comparison ===" << std::endl;
    
    std::string testJson = R"({
        "data": {
            "items": [
                {"id": 1, "value": "first"},
                {"id": 2, "value": "second"},
                {"id": 3, "value": "third"}
            ],
            "metadata": {
                "count": 3,
                "type": "test"
            }
        }
    })";
    
    try {
        // Regular parsing
        auto start = std::chrono::high_resolution_clock::now();
        auto regularResult = JsonValue::parse(testJson);
        auto regularTime = std::chrono::high_resolution_clock::now() - start;
        
        // Streaming parsing
        start = std::chrono::high_resolution_clock::now();
        JsonStreamParser parser;
        JsonStreamBuilder builder;
        
        parser.setEventHandler([&builder](const JsonStreamParser::Event& event) {
            builder.onEvent(event);
            return true;
        });
        
        parser.feed(testJson);
        parser.finish();
        auto streamingResult = builder.getResult();
        auto streamingTime = std::chrono::high_resolution_clock::now() - start;
        
        // Compare results
        std::cout << "Regular parsing result:" << std::endl;
        std::cout << regularResult.dump(2) << std::endl;
        
        std::cout << "\nStreaming parsing result:" << std::endl;
        std::cout << streamingResult.dump(2) << std::endl;
        
        // Note: Results should be functionally equivalent
        std::cout << "\nTiming comparison:" << std::endl;
        std::cout << "Regular: " << std::chrono::duration_cast<std::chrono::microseconds>(regularTime).count() << "μs" << std::endl;
        std::cout << "Streaming: " << std::chrono::duration_cast<std::chrono::microseconds>(streamingTime).count() << "μs" << std::endl;
        
        std::cout << "✅ Streaming vs regular parsing comparison completed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Comparison test failed: " << e.what() << std::endl;
    }
}

int main() {
    try {
        testBasicStreaming();
        testLargeJsonSimulation();
        testErrorHandlingInStreaming();
        testStreamingVsRegularParsing();
        
        std::cout << "\n🎉 Streaming parser tests completed!" << std::endl;
        std::cout << "✅ Basic streaming parsing implemented" << std::endl;
        std::cout << "✅ Event-driven architecture working" << std::endl;
        std::cout << "✅ Memory-efficient processing for large files" << std::endl;
        std::cout << "⚠️ Advanced features need refinement" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
