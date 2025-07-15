# 测试框架的CMakeLists.txt
# 路径相对于项目根目录

# 添加测试框架库
add_library(test_framework 
    test_framework/test_framework.cpp
    test_framework/test_framework.h
)
target_include_directories(test_framework PUBLIC test_framework)
target_compile_features(test_framework PUBLIC cxx_std_17)

# 核心功能测试 (原迁移文件)
add_executable(test_core_functionality tests/test_core_functionality.cpp)
target_link_libraries(test_core_functionality PRIVATE test_framework json_struct)
target_compile_features(test_core_functionality PRIVATE cxx_std_17)

# JSON自动化测试 (原迁移文件)
add_executable(test_json_auto tests/test_json_auto.cpp)
target_link_libraries(test_json_auto PRIVATE test_framework json_struct)
target_compile_features(test_json_auto PRIVATE cxx_std_17)

# Qt类型测试 (原迁移文件)
add_executable(test_qt_types_new tests/test_qt_types_new.cpp)
target_link_libraries(test_qt_types_new PRIVATE test_framework json_struct)
target_compile_features(test_qt_types_new PRIVATE cxx_std_17)

# 增强功能测试 (原迁移文件)
add_executable(test_enhanced_features tests/test_enhanced_features.cpp)
target_link_libraries(test_enhanced_features PRIVATE test_framework json_struct)
target_compile_features(test_enhanced_features PRIVATE cxx_std_17)

# 自定义类型测试 (原迁移文件)
add_executable(test_custom_types tests/test_custom_types.cpp)
target_link_libraries(test_custom_types PRIVATE test_framework json_struct)
target_compile_features(test_custom_types PRIVATE cxx_std_17)

# JSON解析测试 (原迁移文件)
add_executable(test_json_parsing tests/test_json_parsing.cpp)
target_link_libraries(test_json_parsing PRIVATE test_framework json_struct)
target_compile_features(test_json_parsing PRIVATE cxx_std_17)

# 错误恢复测试 (原迁移文件)
add_executable(test_error_recovery tests/test_error_recovery.cpp)
target_link_libraries(test_error_recovery PRIVATE test_framework json_struct)
target_compile_features(test_error_recovery PRIVATE cxx_std_17)

# 性能结构体测试 (原迁移文件)
add_executable(test_performance_struct tests/test_performance_struct.cpp)
target_link_libraries(test_performance_struct PRIVATE test_framework json_struct)
target_compile_features(test_performance_struct PRIVATE cxx_std_17)

# 特殊数字测试 (原迁移文件)
add_executable(test_special_numbers tests/test_special_numbers.cpp)
target_link_libraries(test_special_numbers PRIVATE test_framework json_struct)
target_compile_features(test_special_numbers PRIVATE cxx_std_17)

# JSON指针测试 (原迁移文件)
add_executable(test_json_pointer tests/test_json_pointer.cpp)
target_link_libraries(test_json_pointer PRIVATE test_framework json_struct)
target_compile_features(test_json_pointer PRIVATE cxx_std_17)

# 基础功能测试
add_executable(test_basic_functionality tests/test_basic_functionality.cpp)
target_link_libraries(test_basic_functionality PRIVATE test_framework json_struct)
target_compile_features(test_basic_functionality PRIVATE cxx_std_17)

# 复杂结构测试
add_executable(test_complex_structures tests/test_complex_structures.cpp)
target_link_libraries(test_complex_structures PRIVATE test_framework json_struct)
target_compile_features(test_complex_structures PRIVATE cxx_std_17)

# 错误处理测试
add_executable(test_error_handling tests/test_error_handling.cpp)
target_link_libraries(test_error_handling PRIVATE test_framework json_struct)
target_compile_features(test_error_handling PRIVATE cxx_std_17)

# 测试运行器
add_executable(test_runner tests/test_runner.cpp)
target_link_libraries(test_runner PRIVATE test_framework json_struct)
target_compile_features(test_runner PRIVATE cxx_std_17)

# 复杂嵌套容器测试
add_executable(test_complex_nested_containers tests/test_complex_nested_containers.cpp)
target_link_libraries(test_complex_nested_containers PRIVATE test_framework json_struct)
target_compile_features(test_complex_nested_containers PRIVATE cxx_std_17)

# 全自动测试运行器
add_executable(test_all_runner tests/test_all_runner.cpp)
target_compile_features(test_all_runner PRIVATE cxx_std_17)

# 创建运行核心测试的目标
add_custom_target(run_core_tests
    DEPENDS test_core_functionality test_json_auto test_enhanced_features 
            test_custom_types test_json_parsing test_error_recovery 
            test_performance_struct test_special_numbers test_json_pointer
    COMMENT "Running core migrated tests"
)

# 创建构建所有测试的目标
add_custom_target(build_all_tests
    DEPENDS test_core_functionality test_json_auto test_qt_types_new
            test_enhanced_features test_custom_types test_json_parsing 
            test_error_recovery test_performance_struct test_special_numbers 
            test_json_pointer test_basic_functionality test_complex_structures 
            test_error_handling test_complex_nested_containers test_all_runner
    COMMENT "Building all test executables"
)

# 创建运行全部测试的目标
add_custom_target(run_all_tests
    COMMAND test_all_runner
    DEPENDS build_all_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running all tests automatically"
)
