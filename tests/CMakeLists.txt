# tests/CMakeLists.txt
# 添加Qt支持
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

add_executable(test_precision_fix_en tests/test_precision_fix_en.cpp)
target_link_libraries(test_precision_fix_en json_struct)
target_compile_features(test_precision_fix_en PRIVATE cxx_std_17)

add_executable(test_json_parsing tests/test_json_parsing.cpp)
target_link_libraries(test_json_parsing json_struct)
target_compile_features(test_json_parsing PRIVATE cxx_std_17)

add_executable(test_special_numbers tests/test_special_numbers.cpp)
target_link_libraries(test_special_numbers json_struct)
target_compile_features(test_special_numbers PRIVATE cxx_std_17)

add_executable(test_error_recovery tests/test_error_recovery.cpp)
target_link_libraries(test_error_recovery json_struct)
target_compile_features(test_error_recovery PRIVATE cxx_std_17)

add_executable(test_streaming tests/test_streaming.cpp)
target_link_libraries(test_streaming json_struct)
target_compile_features(test_streaming PRIVATE cxx_std_17)

add_executable(test_jsonpath_unified tests/test_jsonpath.cpp)
target_link_libraries(test_jsonpath_unified json_struct)
target_compile_features(test_jsonpath_unified PRIVATE cxx_std_17)

add_executable(test_enhanced_features tests/test_enhanced_features.cpp)
target_link_libraries(test_enhanced_features json_struct)
target_compile_features(test_enhanced_features PRIVATE cxx_std_17)

add_executable(test_comprehensive_demo tests/test_comprehensive_demo.cpp)
target_link_libraries(test_comprehensive_demo json_struct)
target_compile_features(test_comprehensive_demo PRIVATE cxx_std_17)

add_executable(test_type_registry_core tests/test_type_registry_core.cpp)
target_link_libraries(test_type_registry_core json_struct)
target_compile_features(test_type_registry_core PRIVATE cxx_std_17)

add_executable(test_json_auto tests/test_json_auto.cpp)
target_link_libraries(test_json_auto json_struct)
target_compile_features(test_json_auto PRIVATE cxx_std_17)

add_executable(test_comprehensive_json_auto tests/test_comprehensive_json_auto.cpp)
target_link_libraries(test_comprehensive_json_auto json_struct)
target_compile_features(test_comprehensive_json_auto PRIVATE cxx_std_17)

add_executable(test_core_functionality tests/test_core_functionality.cpp)
target_link_libraries(test_core_functionality json_struct)
target_compile_features(test_core_functionality PRIVATE cxx_std_17)

add_executable(test_advanced_functionality tests/test_advanced_functionality.cpp)
target_link_libraries(test_advanced_functionality json_struct)
target_compile_features(test_advanced_functionality PRIVATE cxx_std_17)

add_executable(test_basic_macros tests/test_basic_macros.cpp)
target_link_libraries(test_basic_macros json_struct)
target_compile_features(test_basic_macros PRIVATE cxx_std_17)

add_executable(test_debug_serialization tests/test_debug_serialization.cpp)
target_link_libraries(test_debug_serialization json_struct)
target_compile_features(test_debug_serialization PRIVATE cxx_std_17)

add_executable(test_performance_struct tests/test_performance_struct.cpp)
target_link_libraries(test_performance_struct json_struct)
target_compile_features(test_performance_struct PRIVATE cxx_std_17)

add_executable(test_performance_pattern tests/test_performance_pattern.cpp)
target_link_libraries(test_performance_pattern json_struct)
target_compile_features(test_performance_pattern PRIVATE cxx_std_17)

add_executable(test_custom_types tests/test_custom_types.cpp)
target_link_libraries(test_custom_types json_struct)
target_compile_features(test_custom_types PRIVATE cxx_std_17)

add_executable(test_json_filter_comprehensive tests/test_json_filter_comprehensive.cpp)
target_link_libraries(test_json_filter_comprehensive json_struct)
target_compile_features(test_json_filter_comprehensive PRIVATE cxx_std_17)

add_executable(test_integration_jsonpath_filter tests/test_integration_jsonpath_filter.cpp)
target_link_libraries(test_integration_jsonpath_filter json_struct)
target_compile_features(test_integration_jsonpath_filter PRIVATE cxx_std_17)

add_executable(test_jsonpath_filter_expression tests/test_jsonpath_filter_expression.cpp)
target_link_libraries(test_jsonpath_filter_expression json_struct)
target_compile_features(test_jsonpath_filter_expression PRIVATE cxx_std_17)

add_executable(test_streaming_query tests/test_streaming_query.cpp)
target_link_libraries(test_streaming_query json_struct)
target_compile_features(test_streaming_query PRIVATE cxx_std_17)

add_executable(test_json_pointer tests/test_json_pointer.cpp)
target_link_libraries(test_json_pointer json_struct)
target_compile_features(test_json_pointer PRIVATE cxx_std_17)

add_executable(json_struct_tests tests/test_qt_types.cpp)
target_link_libraries(json_struct_tests PRIVATE json_struct)
target_compile_features(json_struct_tests PRIVATE cxx_std_17)

target_link_libraries(json_struct_tests PRIVATE Qt6::Core Qt6::Widgets)

# Create custom target for all tests
add_custom_target(tests
    DEPENDS
        test_precision_fix_en
        test_json_parsing
        test_special_numbers
        test_error_recovery
        test_streaming
        test_jsonpath_unified
        test_enhanced_features
        test_comprehensive_demo
        test_type_registry_core
        test_json_auto
        test_comprehensive_json_auto
        test_core_functionality
        test_advanced_functionality
        test_basic_macros
        test_debug_serialization
        test_performance_struct
        test_performance_pattern
        test_custom_types
        test_json_filter_comprehensive
        test_integration_jsonpath_filter
        test_jsonpath_filter_expression
        test_streaming_query
        test_json_pointer
        json_struct_tests
)
