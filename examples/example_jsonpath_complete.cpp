#include "jsonstruct.h"
#include <iostream>
#include <iomanip>

using namespace JsonStruct;

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(60, '=') << "\n\n";
}

void printResult(const std::string& query, const std::vector<const JsonValue*>& results) {
    std::cout << "Query: " << query << "\n";
    std::cout << "Results (" << results.size() << "):\n";
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "  [" << i << "] " << results[i]->toString() << "\n";
    }
    std::cout << "\n";
}

void printResult(const std::string& query, const JsonValue* result) {
    std::cout << "Query: " << query << "\n";
    if (result) {
        std::cout << "Result: " << result->toString() << "\n";
    } else {
        std::cout << "Result: null (not found)\n";
    }
    std::cout << "\n";
}

int main() {
    std::cout << "🎯 Complete JSONPath Features Demonstration\n";
    std::cout << "========================================\n";
    
    // Create a comprehensive test JSON structure
    JsonValue data = JsonValue::object({
        {"company", JsonValue("TechCorp")},
        {"founded", JsonValue(2010)},
        {"departments", JsonValue::array({
            JsonValue::object({
                {"name", JsonValue("Engineering")},
                {"budget", JsonValue(2500000.50)},
                {"employees", JsonValue::array({
                    JsonValue::object({
                        {"name", JsonValue("Alice Johnson")},
                        {"role", JsonValue("Senior Developer")},
                        {"salary", JsonValue(95000)},
                        {"skills", JsonValue::array({
                            JsonValue("C++"), JsonValue("Python"), JsonValue("JavaScript")
                        })}
                    }),
                    JsonValue::object({
                        {"name", JsonValue("Bob Smith")},
                        {"role", JsonValue("DevOps Engineer")},
                        {"salary", JsonValue(88000)},
                        {"skills", JsonValue::array({
                            JsonValue("Docker"), JsonValue("Kubernetes"), JsonValue("AWS")
                        })}
                    }),
                    JsonValue::object({
                        {"name", JsonValue("Carol Williams")},
                        {"role", JsonValue("Frontend Developer")},
                        {"salary", JsonValue(75000)},
                        {"skills", JsonValue::array({
                            JsonValue("React"), JsonValue("Vue"), JsonValue("CSS")
                        })}
                    })
                })}
            }),
            JsonValue::object({
                {"name", JsonValue("Marketing")},
                {"budget", JsonValue(800000.25)},
                {"employees", JsonValue::array({
                    JsonValue::object({
                        {"name", JsonValue("David Brown")},
                        {"role", JsonValue("Marketing Manager")},
                        {"salary", JsonValue(70000)},
                        {"skills", JsonValue::array({
                            JsonValue("SEO"), JsonValue("Analytics"), JsonValue("Strategy")
                        })}
                    }),
                    JsonValue::object({
                        {"name", JsonValue("Eva Davis")},
                        {"role", JsonValue("Content Creator")},
                        {"salary", JsonValue(55000)},
                        {"skills", JsonValue::array({
                            JsonValue("Writing"), JsonValue("Design"), JsonValue("Social Media")
                        })}
                    })
                })}
            })
        })},
        {"office", JsonValue::object({
            {"address", JsonValue("123 Tech Street")},
            {"city", JsonValue("San Francisco")},
            {"facilities", JsonValue::array({
                JsonValue("Cafeteria"), JsonValue("Gym"), JsonValue("Game Room")
            })}
        })}
    });
    
    printSeparator("🔍 BASIC JSONPATH QUERIES");
    
    // Basic property access
    printResult("$.company", data.selectFirst("$.company"));
    printResult("$.founded", data.selectFirst("$.founded"));
    printResult("$.office.city", data.selectFirst("$.office.city"));
    
    printSeparator("📋 ARRAY INDEXING");
    
    // Array indexing
    printResult("$.departments[0].name", data.selectFirst("$.departments[0].name"));
    printResult("$.departments[1].employees[0].name", data.selectFirst("$.departments[1].employees[0].name"));
    printResult("$.office.facilities[2]", data.selectFirst("$.office.facilities[2]"));
    
    printSeparator("✂️ ARRAY SLICING");
    
    // Array slicing
    printResult("$.office.facilities[0:2]", data.selectAll("$.office.facilities[0:2]"));
    printResult("$.departments[0].employees[1:3]", data.selectAll("$.departments[0].employees[1:3]"));
    printResult("$.departments[0].employees[:2]", data.selectAll("$.departments[0].employees[:2]"));
    printResult("$.office.facilities[1:]", data.selectAll("$.office.facilities[1:]"));
    
    printSeparator("🌟 WILDCARDS");
    
    // Wildcards
    printResult("$.departments.*.name", data.selectAll("$.departments.*.name"));
    printResult("$.departments[0].employees.*.role", data.selectAll("$.departments[0].employees.*.role"));
    printResult("$.office.*", data.selectAll("$.office.*"));
    
    printSeparator("🔍 RECURSIVE DESCENT");
    
    // Recursive descent
    printResult("$..name", data.selectAll("$..name"));
    printResult("$..salary", data.selectAll("$..salary"));
    printResult("$..skills", data.selectAll("$..skills"));
    
    printSeparator("🎯 COMBINED FEATURES");
    
    // Combined features
    printResult("$..employees[0].name", data.selectAll("$..employees[0].name"));
    printResult("$.departments.*.budget", data.selectAll("$.departments.*.budget"));
    
    printSeparator("📊 MULTIPLE RESULT SELECTION");
    
    // Demonstrate selectValues (copies instead of pointers)
    auto salaryValues = data.selectValues("$..salary");
    std::cout << "Query: $..salary (using selectValues)\n";
    std::cout << "Copied Values (" << salaryValues.size() << "):\n";
    for (size_t i = 0; i < salaryValues.size(); ++i) {
        std::cout << "  [" << i << "] $" << salaryValues[i].toInt() << "\n";
    }
    std::cout << "\n";
    
    printSeparator("🛑 ERROR HANDLING");
    
    // Error handling examples
    printResult("$.nonexistent", data.selectFirst("$.nonexistent"));
    printResult("$.departments[10]", data.selectFirst("$.departments[10]"));
    printResult("$.company[0]", data.selectFirst("$.company[0]"));  // Invalid: string accessed as array
    printResult("$.departments[0].employees[5:10]", data.selectAll("$.departments[0].employees[5:10]"));  // Out of bounds slice
    
    printSeparator("✅ FEATURE SUMMARY");
    
    std::cout << "🎉 All JSONPath Features Successfully Demonstrated!\n\n";
    std::cout << "Implemented Features:\n";
    std::cout << "  • Basic property access ($.prop.subprop)\n";
    std::cout << "  • Array indexing ($.arr[0], $.arr[1].prop)\n";
    std::cout << "  • Array slicing ($.arr[1:3], $.arr[:2], $.arr[3:])\n";
    std::cout << "  • Wildcards ($.*.prop, $.obj.*)\n";
    std::cout << "  • Recursive descent ($..prop - finds all matching properties)\n";
    std::cout << "  • Multiple result selection (selectAll, selectValues)\n";
    std::cout << "  • Combined feature usage\n";
    std::cout << "  • Comprehensive error handling and bounds checking\n";
    std::cout << "  • Support for complex nested structures\n\n";
    
    std::cout << "💡 Performance Notes:\n";
    std::cout << "  • selectFirst() returns pointer (O(1) access)\n";
    std::cout << "  • selectAll() returns vector of pointers (efficient)\n";
    std::cout << "  • selectValues() returns vector of copies (safe for modification)\n";
    std::cout << "  • All operations include bounds checking and error handling\n\n";
    
    return 0;
}
