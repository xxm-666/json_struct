# JsonStruct Registry - Enterprise-Grade JSON Library for C++17+

> A modern, high-performance JSON library for C++17+ with advanced features, type safety, and enterprise-grade capabilities.

## � Project Status: Migration Complete

**✅ MIGRATION SUCCESSFULLY COMPLETED**
- All JsonValueEnhanced functionality has been migrated to unified JsonValue API
- Legacy implementations removed
- All tests passing
- Build system updated
- Full C++17+ feature support maintained

## 📁 Project Structure

```
jsonstruct_registry/
├── src/                           # Core Source Code
│   ├── json_value.h               # Core JsonValue Header
│   ├── json_value.cpp             # Core JsonValue Implementation
│   ├── json_number.h              # High-Precision Number Handling
│   ├── json_stream_parser.h       # Streaming Parser
│   ├── json_path.h                # JSONPath Query Header
│   ├── json_path.cpp              # JSONPath Query Implementation
│   ├── std/                       # STL Integration
│   │   ├── jsonstruct_std.h       # STL Main Header
│   │   ├── std_jsonstruct.h       # STL Serialization Support
│   │   ├── std_types_registration.h # STL Type Registration
│   │   ├── std_type_registry.h    # Type Registry System
│   │   └── std_custom_types_example.h # Custom Types Example
│   └── qt/                        # Qt Integration (Optional)
│       └── qt_types_registration.h
├── tests/                         # Test Programs
│   ├── test_std_system.cpp        # STL System Tests
│   ├── test_enhanced_features.cpp # Advanced Features Tests
│   ├── test_comprehensive_demo.cpp# Comprehensive Feature Demo
│   ├── test_streaming.cpp         # Streaming Parser Tests
│   ├── test_json_parsing.cpp      # JSON Parsing Tests
│   ├── test_jsonpath*.cpp         # JSONPath Test Suite
│   ├── test_precision_fix*.cpp    # Numeric Precision Tests
│   ├── test_special_numbers.cpp   # Special Numbers Tests
│   └── test_error_recovery.cpp    # Error Recovery Tests
├── examples/                      # Example Programs
│   ├── demo_enhanced_simple.cpp   # Basic Feature Demo
│   ├── demo_jsonpath_complete.cpp # Complete JSONPath Demo
│   ├── demo_precision_fix*.cpp    # Numeric Precision Demo
│   ├── demo_migration_compatibility*.cpp # API Compatibility Demo
│   └── debug_recursive.cpp        # Debug Tools
├── docs/                          # Documentation
│   ├── QUICK_START.md             # Quick Start Guide
│   ├── API_REFERENCE.md           # API Reference Manual
│   ├── USER_GUIDE.md              # User Guide
│   ├── ADVANCED_FEATURES.md       # Advanced Features
│   └── PERFORMANCE_GUIDE.md       # Performance Optimization Guide
├── build/                         # Build Output
├── CMakeLists.txt                 # CMake Build Configuration
└── README.md                      # This file
```
## 🚀 Quick Start

### Basic Usage

```cpp
#include "jsonstruct_std.h"
using namespace JsonStruct;

int main() {
    // Create JSON object
    JsonValue json;
    json["name"] = "Alice";
    json["age"] = 30;
    json["skills"] = JsonValue::ArrayType{"C++", "JSON", "Design"};
    
    // Serialize
    std::string jsonString = json.serialize(2); // With indentation
    std::cout << jsonString << std::endl;
    
    // Parse
    JsonValue parsed = JsonValue::parse(jsonString);
    std::cout << "Name: " << parsed["name"].toString() << std::endl;
    
    return 0;
}
```

### Build Project

```bash
# Configure CMake
cmake -B build -S .

# Build all targets
cmake --build build --config Release

# Run tests
./build/Release/test_std_system        # Core functionality tests
./build/Release/demo_enhanced_simple   # Basic feature demo
```

## 🏆 Core Features

### 🔢 High-Precision Numeric Support
- **JsonNumber class**: Support for large integers beyond IEEE 754 precision limits
- **Type detection**: Intelligent distinction between integers and floating-point numbers
- **Special value support**: Complete NaN/Infinity handling

### 🌟 Modern C++17+ Features
- **Type safety**: std::variant ensures runtime type safety
- **Modern syntax**: std::optional, std::string_view, perfect forwarding
- **Visitor pattern**: Type-safe value access mechanism
- **Move semantics**: Efficient operations for large objects

### 🔍 Enterprise JSONPath (100% Complete)
- **Full JSONPath support**: Complete implementation of JSONPath query language
- **Advanced queries**: Array indexing, slicing, wildcards, recursive descent
- **Multiple selection**: selectFirst(), selectAll(), selectValues() methods
- **Path validation**: pathExists() for existence checking
- **Performance optimized**: Efficient path parsing and value extraction

**JSONPath Examples:**
```cpp
JsonValue data = JsonValue::parse(R"({
    "books": [
        {"title": "C++ Guide", "price": 29.99},
        {"title": "JSON Manual", "price": 19.99}
    ]
})");

// Array indexing
auto title = data.selectFirst("$.books[0].title");        // "C++ Guide"

// Array wildcards  
auto titles = data.selectAll("$.books[*].title");         // All book titles

// Array slicing
auto subset = data.selectAll("$.books[0:2]");             // First 2 books

// Recursive descent
auto prices = data.selectAll("$..price");                 // All prices
```

### 🌊 Streaming Parser
- **Event-driven**: Memory-efficient processing of large files
- **Error recovery**: Fault-tolerant parsing and character-level recovery
- **Configurable**: Support for comments, trailing commas and other non-standard features

### 🎯 Type Registration System
- **STL containers**: Automatic support for vector, map and other standard containers
- **Custom types**: Simple type registration macros
- **Serialization**: Type-safe bidirectional conversion

## 📚 Documentation

- **[Quick Start](docs/QUICK_START.md)** - Basic examples and quick start guide
- **[API Reference](docs/API_REFERENCE.md)** - Complete class and method documentation
- **[User Guide](docs/USER_GUIDE.md)** - Detailed usage instructions and examples
- **[Advanced Features](docs/ADVANCED_FEATURES.md)** - JSONPath, streaming parser, type registration, etc.
- **[Performance Guide](docs/PERFORMANCE_GUIDE.md)** - Optimization recommendations and best practices

## 🎯 Use Cases

**Ideal choice for**:
- 🏢 **Enterprise applications**: Production environments requiring high reliability and performance
- 🔢 **Precision-sensitive applications**: Finance, scientific computing scenarios requiring numerical precision
- 📊 **Big data processing**: Streaming parsing of large JSON files
- 🔍 **Complex queries**: Applications requiring advanced query features like JSONPath
- 🚀 **Modern C++ projects**: Projects leveraging C++17+ new features

## 📈 Performance Features

- **O(1) lookup**: Fast key-value lookup using std::unordered_map
- **Move semantics**: Zero-copy optimization for large object operations
- **Memory efficient**: Streaming parser uses only O(depth) memory
- **Compile-time optimization**: Extensive use of templates and constexpr

## 📝 License

This project is licensed under the MIT License. See the LICENSE file in the project root for details.

## 🤝 Contributing

Issues and Pull Requests are welcome to improve this project.

---

**JsonStruct Registry** - Enterprise-grade C++17+ JSON processing library with modern design, high performance, production-ready.
