cmake_minimum_required(VERSION 3.16)
project(json_struct LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 数值精度专项测试程序
add_executable(test_numeric_precision
    json_number.h
    json_value_enhanced.h
    json_value_enhanced.cpp
    test_numeric_precision.cpp
)
target_compile_features(test_numeric_precision PRIVATE cxx_std_17)

# 增强版测试程序
add_executable(test_enhanced_features
    json_number.h
    json_value_enhanced.h
    json_value_enhanced.cpp
    test_enhanced_features.cpp
)
target_compile_features(test_enhanced_features PRIVATE cxx_std_17)
