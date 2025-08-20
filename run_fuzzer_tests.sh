#!/bin/bash

# JSON Struct Fuzzer Test Suite
# 这个脚本提供了多种fuzzer测试选项

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
FUZZER_EXECUTABLE="$BUILD_DIR/tests/fuzzing/json_fuzzer"
ADVANCED_FUZZER_EXECUTABLE="$BUILD_DIR/tests/fuzzing/advanced_json_fuzzer"
SEED_CORPUS_DIR="$SCRIPT_DIR/tests/fuzzing/seed_corpus"
OUTPUT_DIR="$SCRIPT_DIR/fuzzer_output"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

echo_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

echo_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

echo_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

# 检查构建
check_build() {
    if [ ! -f "$FUZZER_EXECUTABLE" ]; then
        echo_error "Basic fuzzer executable not found at $FUZZER_EXECUTABLE"
        echo_info "Building basic fuzzer..."
        cd "$SCRIPT_DIR"
        cmake --build build --target json_fuzzer -j4
        if [ ! -f "$FUZZER_EXECUTABLE" ]; then
            echo_error "Failed to build basic fuzzer"
            exit 1
        fi
    fi
    
    if [ ! -f "$ADVANCED_FUZZER_EXECUTABLE" ]; then
        echo_error "Advanced fuzzer executable not found at $ADVANCED_FUZZER_EXECUTABLE"
        echo_info "Building advanced fuzzer..."
        cd "$SCRIPT_DIR"
        cmake --build build --target advanced_json_fuzzer -j4
        if [ ! -f "$ADVANCED_FUZZER_EXECUTABLE" ]; then
            echo_error "Failed to build advanced fuzzer"
            exit 1
        fi
    fi
    
    echo_success "Both fuzzer executables found"
}

# 创建输出目录
create_output_dir() {
    mkdir -p "$OUTPUT_DIR"
    echo_info "Output directory: $OUTPUT_DIR"
}

# 快速健康检查（100次迭代）
quick_test() {
    echo_info "Running quick health check (100 iterations)..."
    local log_file="$OUTPUT_DIR/quick_test.log"
    
    if timeout 10s "$FUZZER_EXECUTABLE" -max_len=512 -timeout=1 -runs=100 > "$log_file" 2>&1; then
        echo_success "Quick test passed"
        grep "Done 100 runs" "$log_file" | tail -1
    else
        echo_error "Quick test failed"
        tail -20 "$log_file"
        return 1
    fi
}

# 中等强度测试（10,000次迭代）
medium_test() {
    echo_info "Running medium intensity test (10,000 iterations)..."
    local log_file="$OUTPUT_DIR/medium_test.log"
    
    echo_info "This may take several minutes..."
    if timeout 300s "$FUZZER_EXECUTABLE" -max_len=1024 -timeout=3 -runs=10000 > "$log_file" 2>&1; then
        echo_success "Medium test completed"
        grep "Done 10000 runs" "$log_file" | tail -1
    else
        echo_warn "Medium test timed out or interrupted"
        tail -10 "$log_file"
    fi
}

# 高级fuzzer测试
advanced_test() {
    echo_info "Running advanced fuzzer test (2,000 iterations)..."
    local log_file="$OUTPUT_DIR/advanced_test.log"
    
    echo_info "This tests complex JSON structures and edge cases..."
    if timeout 120s "$ADVANCED_FUZZER_EXECUTABLE" -max_len=1024 -timeout=5 -runs=2000 > "$log_file" 2>&1; then
        echo_success "Advanced test completed"
        grep "Done 2000 runs" "$log_file" | tail -1
        # 显示代码覆盖率信息
        grep "cov:" "$log_file" | tail -1
    else
        echo_warn "Advanced test timed out or interrupted"
        tail -10 "$log_file"
    fi
}
corpus_test() {
    echo_info "Running corpus-based test..."
    local corpus_dir="$OUTPUT_DIR/corpus"
    local log_file="$OUTPUT_DIR/corpus_test.log"
    
    # 复制种子语料库
    rm -rf "$corpus_dir"
    cp -r "$SEED_CORPUS_DIR" "$corpus_dir"
    echo_info "Copied seed corpus to $corpus_dir"
    
    echo_info "Running with corpus for 60 seconds..."
    if timeout 60s "$FUZZER_EXECUTABLE" "$corpus_dir" -max_len=2048 -timeout=5 > "$log_file" 2>&1; then
        echo_success "Corpus test completed"
    else
        echo_warn "Corpus test timed out or interrupted"
    fi
    
    # 显示结果
    tail -10 "$log_file"
    echo_info "Generated corpus files: $(ls -1 "$corpus_dir" | wc -l)"
}

# 高强度连续测试（用户控制停止）
stress_test() {
    echo_info "Running stress test (continuous until Ctrl+C)..."
    local log_file="$OUTPUT_DIR/stress_test.log"
    local corpus_dir="$OUTPUT_DIR/stress_corpus"
    
    # 准备语料库
    rm -rf "$corpus_dir"
    cp -r "$SEED_CORPUS_DIR" "$corpus_dir"
    
    echo_warn "This will run continuously. Press Ctrl+C to stop."
    echo_info "Log file: $log_file"
    echo_info "Corpus directory: $corpus_dir"
    
    echo_info "Starting in 3 seconds..."
    sleep 3
    
    "$FUZZER_EXECUTABLE" "$corpus_dir" -max_len=4096 -timeout=10 2>&1 | tee "$log_file"
}

# 检查crasher文件
check_crashes() {
    echo_info "Checking for crash files..."
    local crash_files=$(find "$SCRIPT_DIR" -name "crash-*" -o -name "leak-*" -o -name "timeout-*" 2>/dev/null)
    
    if [ -z "$crash_files" ]; then
        echo_success "No crash files found"
    else
        echo_warn "Found potential crash files:"
        echo "$crash_files"
        echo_info "To reproduce a crash: $FUZZER_EXECUTABLE <crash_file>"
    fi
}

# 清理函数
cleanup() {
    echo_info "Cleaning up old test artifacts..."
    find "$SCRIPT_DIR" -name "crash-*" -delete 2>/dev/null || true
    find "$SCRIPT_DIR" -name "leak-*" -delete 2>/dev/null || true
    find "$SCRIPT_DIR" -name "timeout-*" -delete 2>/dev/null || true
    rm -rf "$OUTPUT_DIR"
    echo_success "Cleanup completed"
}

# 显示帮助
show_help() {
    cat << EOF
JSON Struct Fuzzer Test Suite

Usage: $0 [COMMAND]

Commands:
    quick       Run quick health check (100 iterations, ~10 seconds)
    medium      Run medium intensity test (10,000 iterations, ~5 minutes)
    advanced    Run advanced fuzzer test (2,000 iterations with complex structures)
    corpus      Run corpus-based test (60 seconds with seed corpus)
    stress      Run continuous stress test (until Ctrl+C)
    check       Check for crash files
    cleanup     Clean up test artifacts
    all         Run quick, medium, advanced, and corpus tests
    help        Show this help message

Examples:
    $0 quick           # Quick test
    $0 stress          # Stress test (Ctrl+C to stop)
    $0 all             # Run all automated tests
    $0 cleanup         # Clean up artifacts

Output location: $OUTPUT_DIR
EOF
}

# 主逻辑
main() {
    local command="${1:-help}"
    
    echo_info "JSON Struct Fuzzer Test Suite"
    echo_info "=============================="
    
    case "$command" in
        quick)
            check_build
            create_output_dir
            quick_test
            check_crashes
            ;;
        medium)
            check_build
            create_output_dir
            medium_test
            check_crashes
            ;;
        advanced)
            check_build
            create_output_dir
            advanced_test
            check_crashes
            ;;
        corpus)
            check_build
            create_output_dir
            corpus_test
            check_crashes
            ;;
        stress)
            check_build
            create_output_dir
            stress_test
            check_crashes
            ;;
        check)
            check_crashes
            ;;
        cleanup)
            cleanup
            ;;
        all)
            check_build
            create_output_dir
            echo_info "Running comprehensive test suite..."
            quick_test && medium_test && advanced_test && corpus_test
            check_crashes
            echo_success "All tests completed!"
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            echo_error "Unknown command: $command"
            show_help
            exit 1
            ;;
    esac
}

# 运行主函数
main "$@"