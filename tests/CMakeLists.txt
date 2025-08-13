# ============================================================================
# 核心功能测试
# ============================================================================

# 核心功能和类型系统测试
add_executable(test_core_functionality test_core_functionality.cpp)
target_link_libraries(test_core_functionality PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_core_functionality PRIVATE cxx_std_17)

# JSON自动化测试
add_executable(test_json_auto test_json_auto.cpp)
target_link_libraries(test_json_auto PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_json_auto PRIVATE cxx_std_17)

# 自定义类型测试
add_executable(test_custom_types test_custom_types.cpp)
target_link_libraries(test_custom_types PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_custom_types PRIVATE cxx_std_17)

# ============================================================================
# JSON解析和数据处理测试
# ============================================================================

# JSON解析测试
add_executable(test_json_parsing test_json_parsing.cpp)
target_link_libraries(test_json_parsing PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_json_parsing PRIVATE cxx_std_17)

# JSON指针测试（原JSON Path Basic，实际测试JSON Pointer）
add_executable(test_json_pointer_basic test_json_pointer_basic.cpp)
target_link_libraries(test_json_pointer_basic PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_json_pointer_basic PRIVATE cxx_std_17)

# JSON指针标准测试
add_executable(test_json_pointer test_json_pointer.cpp)
target_link_libraries(test_json_pointer PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_json_pointer PRIVATE cxx_std_17)

# JSON流解析器测试
add_executable(test_json_stream_parser test_json_stream_parser.cpp)
target_link_libraries(test_json_stream_parser PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_json_stream_parser PRIVATE cxx_std_17)

# ============================================================================
# JSONPath 功能测试
# ============================================================================

# JSONPath 综合测试（包含所有JSONPath功能）
add_executable(test_jsonpath_comprehensive test_jsonpath_comprehensive.cpp)
target_link_libraries(test_jsonpath_comprehensive PRIVATE TestFramework::test_framework json_struct)
target_include_directories(test_jsonpath_comprehensive PRIVATE json_struct)
target_compile_features(test_jsonpath_comprehensive PRIVATE cxx_std_17)

# JSONPath 可变操作测试
add_executable(test_json_path_mutable test_json_path_mutable.cpp)
target_link_libraries(test_json_path_mutable PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_json_path_mutable PRIVATE cxx_std_17)

# ============================================================================
# 错误处理和边界测试
# ============================================================================

# 错误恢复测试
add_executable(test_error_recovery test_error_recovery.cpp)
target_link_libraries(test_error_recovery PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_error_recovery PRIVATE cxx_std_17)

# 错误处理测试
add_executable(test_error_handling test_error_handling.cpp)
target_link_libraries(test_error_handling PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_error_handling PRIVATE cxx_std_17)

# 错误代码测试
add_executable(test_error_codes test_error_codes.cpp)
target_link_libraries(test_error_codes PRIVATE TestFramework::test_framework json_struct)
target_include_directories(test_error_codes PRIVATE json_struct)
target_compile_features(test_error_codes PRIVATE cxx_std_17)

# 极端异常场景测试
add_executable(test_extreme_exception_scenarios test_extreme_exception_scenarios.cpp)
target_link_libraries(test_extreme_exception_scenarios PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_extreme_exception_scenarios PRIVATE cxx_std_17)

# 类型转换和边界条件测试
add_executable(test_type_conversion_boundary test_type_conversion_boundary.cpp)
target_link_libraries(test_type_conversion_boundary PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_type_conversion_boundary PRIVATE cxx_std_17)

# ============================================================================
# 性能和基准测试
# ============================================================================

# 性能结构体测试
add_executable(test_performance_struct test_performance_struct.cpp)
target_link_libraries(test_performance_struct PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_performance_struct PRIVATE cxx_std_17)

# 性能极限测试
add_executable(test_performance_limits test_performance_limits.cpp)
target_link_libraries(test_performance_limits PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_performance_limits PRIVATE cxx_std_17)

# 惰性性能综合测试
add_executable(test_lazy_performance_comprehensive test_lazy_performance_comprehensive.cpp)
target_link_libraries(test_lazy_performance_comprehensive PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_lazy_performance_comprehensive PRIVATE cxx_std_17)

# ============================================================================
# 复杂结构和大型数据测试
# ============================================================================

# 复杂结构测试
add_executable(test_complex_structures test_complex_structures.cpp)
target_link_libraries(test_complex_structures PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_complex_structures PRIVATE cxx_std_17)

# 复杂嵌套容器测试
add_executable(test_complex_nested_containers test_complex_nested_containers.cpp)
target_link_libraries(test_complex_nested_containers PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_complex_nested_containers PRIVATE cxx_std_17)

# 超大/超深嵌套对象与数组测试
add_executable(test_large_deep_nested_objects test_large_deep_nested_objects.cpp)
target_link_libraries(test_large_deep_nested_objects PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_large_deep_nested_objects PRIVATE cxx_std_17)

# 惰性查询缓存性能测试
add_executable(test_lazy_query_cache_performance test_lazy_query_cache_performance.cpp)
target_link_libraries(test_lazy_query_cache_performance PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_lazy_query_cache_performance PRIVATE cxx_std_17)


# 增强型懒查询综合测试
add_executable(test_lazy_query_comprehensive test_lazy_query_comprehensive.cpp)
target_link_libraries(test_lazy_query_comprehensive PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_lazy_query_comprehensive PRIVATE cxx_std_17)

# ============================================================================
# 特殊功能测试
# ============================================================================

# 特殊数字测试
add_executable(test_special_numbers test_special_numbers.cpp)
target_link_libraries(test_special_numbers PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_special_numbers PRIVATE cxx_std_17)

# 带空格属性名测试
add_executable(test_spaced_properties test_spaced_properties.cpp)
target_link_libraries(test_spaced_properties PRIVATE TestFramework::test_framework json_struct)
target_include_directories(test_spaced_properties PRIVATE json_struct)
target_compile_features(test_spaced_properties PRIVATE cxx_std_17)

# 序列化选项测试
add_executable(test_serialization_basic test_serialization_basic.cpp)
target_link_libraries(test_serialization_basic PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_serialization_basic PRIVATE cxx_std_17)

# 序列化选项扩展测试
add_executable(test_serialization_options test_serialization_options.cpp)
target_link_libraries(test_serialization_options PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_serialization_options PRIVATE cxx_std_17)

# ============================================================================
# 高级功能测试
# ============================================================================

# JSON管道测试
add_executable(test_json_pipeline test_json_pipeline.cpp)
target_link_libraries(test_json_pipeline PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_json_pipeline PRIVATE cxx_std_17)

# JSON查询生成器测试
add_executable(test_json_query_generator test_json_query_generator.cpp)
target_link_libraries(test_json_query_generator PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_json_query_generator PRIVATE cxx_std_17)

# 多线程并发读写测试
add_executable(test_multithreaded_concurrency test_multithreaded_concurrency.cpp)
target_link_libraries(test_multithreaded_concurrency PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_multithreaded_concurrency PRIVATE cxx_std_17)

# ============================================================================
# 平台特定和外部标准测试
# ============================================================================

if(ENABLE_QT_SUPPORT)
# Qt类型测试
add_executable(test_qt_types_new test_qt_types_new.cpp)
target_link_libraries(test_qt_types_new PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_qt_types_new PRIVATE cxx_std_17)
endif()

# RFC 7396 JSON Merge Patch 测试
add_executable(rfc7396_test rfc7396_test.cpp)
target_link_libraries(rfc7396_test PRIVATE TestFramework::test_framework json_struct)
target_include_directories(rfc7396_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_features(rfc7396_test PRIVATE cxx_std_17)

# ============================================================================
# 简单和工具测试
# ============================================================================

# 简单过滤器测试
add_executable(simple_filter_test simple_filter_test.cpp)
target_link_libraries(simple_filter_test PRIVATE TestFramework::test_framework json_struct)
target_compile_features(simple_filter_test PRIVATE cxx_std_17)

# ============================================================================
# 测试运行器
# ============================================================================

# 惰性查询测试套件运行器
add_executable(test_lazy_suite_runner test_lazy_suite_runner.cpp)
target_link_libraries(test_lazy_suite_runner PRIVATE TestFramework::test_framework json_struct)
target_compile_features(test_lazy_suite_runner PRIVATE cxx_std_17)

# 全自动测试运行器
add_executable(test_all_runner test_all_runner.cpp)
target_compile_features(test_all_runner PRIVATE cxx_std_17)

# ============================================================================
# 自定义目标
# ============================================================================

# 创建运行核心测试的目标
add_custom_target(run_core_tests
    DEPENDS test_core_functionality test_json_auto 
            test_custom_types test_json_parsing test_error_recovery 
            test_performance_struct test_special_numbers test_json_pointer
            test_large_deep_nested_objects test_multithreaded_concurrency
            test_extreme_exception_scenarios test_performance_limits
    COMMENT "Running core functionality tests"
)

# 创建运行JSONPath测试的目标
add_custom_target(run_jsonpath_tests
    DEPENDS test_jsonpath_comprehensive test_json_path_mutable
    COMMENT "Running JSONPath tests"
)

# 创建运行性能测试的目标
add_custom_target(run_performance_tests
    DEPENDS test_performance_struct test_performance_limits 
            test_lazy_performance_comprehensive
    COMMENT "Running performance tests"
)

# 创建构建所有测试的目标
add_custom_target(build_all_tests
    DEPENDS test_core_functionality test_json_auto 
            test_custom_types test_json_parsing test_error_recovery 
            test_performance_struct test_special_numbers test_json_pointer_basic
            test_json_pointer test_jsonpath_comprehensive test_json_path_mutable
            test_error_handling test_error_codes test_extreme_exception_scenarios
            test_type_conversion_boundary test_performance_limits 
            test_lazy_performance_comprehensive test_complex_structures 
            test_complex_nested_containers test_large_deep_nested_objects
            test_multithreaded_concurrency test_spaced_properties
            test_serialization_basic test_serialization_options
            test_json_pipeline test_json_query_generator test_lazy_query_comprehensive
            rfc7396_test simple_filter_test
            test_lazy_suite_runner test_all_runner
    COMMENT "Building all test executables" 
)

# 创建运行全部测试的目标
add_custom_target(run_all_tests
    COMMAND test_all_runner
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS build_all_tests
    COMMENT "Running all tests automatically"
)