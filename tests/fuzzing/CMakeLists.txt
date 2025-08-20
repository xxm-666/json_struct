# ============================================================================
# Fuzzing Tests (libFuzzer)
#
# 查找 Fuzzer 并设置编译目标。这需要使用 Clang 编译器。
# ============================================================================

# 检查编译器是否支持 fuzzing
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(WARNING "Fuzzing requires Clang compiler. Current compiler: ${CMAKE_CXX_COMPILER_ID}")
    return()
endif()

message(STATUS "Clang compiler detected. Setting up fuzzing tests.")

# Windows 上的 libFuzzer 可能有链接问题，我们提供两种方案
if(WIN32)
    # 方案1：尝试使用 libFuzzer
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-fsanitize=fuzzer" COMPILER_SUPPORTS_FUZZER)
    
    if(COMPILER_SUPPORTS_FUZZER)
        message(STATUS "Attempting to use libFuzzer on Windows...")
        set(USE_LIBFUZZER TRUE)
    else()
        message(STATUS "libFuzzer not fully supported on Windows, using alternative approach...")
        set(USE_LIBFUZZER FALSE)
    endif()
else()
    # Linux/macOS 应该正常支持 libFuzzer
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-fsanitize=fuzzer" COMPILER_SUPPORTS_FUZZER)
    
    if(NOT COMPILER_SUPPORTS_FUZZER)
        message(WARNING "Compiler does not support -fsanitize=fuzzer flag")
        return()
    endif()
    
    set(USE_LIBFUZZER TRUE)
endif()

# 创建 fuzzer 可执行文件
add_executable(json_fuzzer json_fuzzer.cpp)

if(USE_LIBFUZZER)
    # 使用真正的 libFuzzer
    target_compile_options(json_fuzzer PRIVATE
        -fsanitize=fuzzer,address,undefined
        -g
        -O1
    )
    
    target_link_options(json_fuzzer PRIVATE
        -fsanitize=fuzzer,address,undefined
    )
    
    message(STATUS "Using libFuzzer with sanitizers")
else()
    # Windows 替代方案：创建一个简单的随机测试器
    target_compile_definitions(json_fuzzer PRIVATE FUZZER_NO_LIBFUZZER=1)
    target_compile_options(json_fuzzer PRIVATE
        -g
        -O1
    )
    
    message(STATUS "Using alternative fuzzing approach for Windows")
endif()

# 链接到 json_struct 库
target_link_libraries(json_fuzzer PRIVATE json_struct)

# 设置包含目录
target_include_directories(json_fuzzer PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# 创建运行目标
if(USE_LIBFUZZER)
    # libFuzzer 目标
    add_custom_target(run_fuzzer
        COMMAND $<TARGET_FILE:json_fuzzer> -max_len=2048 -timeout=5 -runs=10000
        DEPENDS json_fuzzer
        COMMENT "Running JSON Fuzzer for 10000 iterations. Press Ctrl+C to stop."
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    add_custom_target(run_fuzzer_continuous
        COMMAND $<TARGET_FILE:json_fuzzer> -max_len=2048 -timeout=10
        DEPENDS json_fuzzer
        COMMENT "Running JSON Fuzzer continuously. Press Ctrl+C to stop."
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    add_custom_target(run_fuzzer_corpus
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/corpus
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/seed_corpus ${CMAKE_CURRENT_BINARY_DIR}/corpus
        COMMAND $<TARGET_FILE:json_fuzzer> ${CMAKE_CURRENT_BINARY_DIR}/corpus -max_len=2048 -timeout=5 -runs=1000
        DEPENDS json_fuzzer
        COMMENT "Running JSON Fuzzer with seed corpus directory"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
else()
    # 简单的测试目标
    add_custom_target(run_fuzzer
        COMMAND $<TARGET_FILE:json_fuzzer>
        DEPENDS json_fuzzer
        COMMENT "Running JSON Fuzzer (Windows alternative mode)"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

message(STATUS "Fuzzer targets created:")
message(STATUS "  - json_fuzzer: The fuzzer executable")
if(USE_LIBFUZZER)
    message(STATUS "  - run_fuzzer: Run fuzzer for 10000 iterations")
    message(STATUS "  - run_fuzzer_continuous: Run fuzzer continuously")
    message(STATUS "  - run_fuzzer_corpus: Run fuzzer with corpus directory")
else()
    message(STATUS "  - run_fuzzer: Run alternative fuzzer")
endif()
