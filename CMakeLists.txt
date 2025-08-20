# =============================================================================
#
#                           JsonStruct Project
#
# =============================================================================
#
# -----------------------------------------------------------------------------
# I. Project Definition & Version
# -----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)

# 使用 project 命令直接设置版本，CMake会自动设置 PROJECT_VERSION 等变量
project(json_struct
    VERSION "1.2.0"
    DESCRIPTION "A powerful C++ library for JSON serialization and deserialization"
    LANGUAGES CXX
)

# -----------------------------------------------------------------------------
# II. Global Configuration & Options
# -----------------------------------------------------------------------------

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # 推荐关闭，使用标准C++

# 定义构建选项
if (NOT DEFINED ENABLE_QT_SUPPORT)
    option(ENABLE_QT_SUPPORT "Enable support for Qt types" OFF)
endif()
if (NOT DEFINED BUILD_TESTS)
    option(BUILD_TESTS "Build the project's test suite" OFF)
endif()
if (NOT DEFINED BUILD_FUZZER)
    option(BUILD_FUZZER "Build fuzzing tests with libFuzzer (requires Clang)" OFF)
endif()

# -----------------------------------------------------------------------------
# III. Build Information (Git & Timestamp)
# -----------------------------------------------------------------------------

# 获取构建时间
string(TIMESTAMP JSON_STRUCT_BUILD_DATE "%Y-%m-%d %H:%M:%S UTC" UTC)

# 获取Git信息（如果可用）
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE JSON_STRUCT_GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE JSON_STRUCT_GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
else()
    set(JSON_STRUCT_GIT_COMMIT "N/A")
    set(JSON_STRUCT_GIT_BRANCH "N/A")
endif()

# -----------------------------------------------------------------------------
# IV. Library Target Definition (json_struct)
# -----------------------------------------------------------------------------

# 将生成的头文件（如version.h）放到一个独立的目录
set(GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated/include")

# -- 配置版本头文件 --
# 1. 为 version.h.in 模板设置它所期望的变量
set(JSON_STRUCT_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(JSON_STRUCT_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(JSON_STRUCT_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(JSON_STRUCT_VERSION_SUFFIX "") # dev, rc1, etc.

# 2. 执行配置
configure_file(
    "src/version.h.in"
    "${GENERATED_INCLUDE_DIR}/version.h"
    @ONLY
)

# 收集核心源文件
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    "src/json_engine/*.cpp"
    "src/json_engine/internal/*.cpp"
    "src/std_types/*.cpp"
    "src/type_registry/*.cpp"
    "src/version.cpp"
)

# 如果启用 Qt 支持，则添加 Qt 相关源文件
if(ENABLE_QT_SUPPORT)
    file(GLOB_RECURSE QT_SOURCES CONFIGURE_DEPENDS
        "src/qt_types/*.cpp"
    )
    list(APPEND CORE_SOURCES ${QT_SOURCES})
endif()

# 创建库
add_library(json_struct STATIC ${CORE_SOURCES})
add_library(json_struct::json_struct ALIAS json_struct) # 添加别名

# 为库目标设置属性
target_compile_features(json_struct PRIVATE cxx_std_17)

# 为 json_struct 添加头文件搜索路径
target_include_directories(json_struct
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${GENERATED_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/json_engine
        ${CMAKE_CURRENT_SOURCE_DIR}/src/json_engine/internal
        ${CMAKE_CURRENT_SOURCE_DIR}/src/std_types
        ${CMAKE_CURRENT_SOURCE_DIR}/src/type_registry
)

# 处理可选的 Qt 支持
if(ENABLE_QT_SUPPORT)
    find_package(Qt6 COMPONENTS Core Widgets)
    if(Qt6_FOUND)
        # --- 诊断信息 ---
        message(STATUS "--- Qt Diagnostics ---")
        message(STATUS "Qt6_FOUND: ${Qt6_FOUND}")
        message(STATUS "Qt6_INCLUDE_DIRS: ${Qt6_INCLUDE_DIRS}")
        get_target_property(QT6_CORE_INCLUDE_DIRS Qt6::Core INTERFACE_INCLUDE_DIRECTORIES)
        message(STATUS "Qt6::Core INTERFACE_INCLUDE_DIRECTORIES: ${QT6_CORE_INCLUDE_DIRS}")
        message(STATUS "--- End Qt Diagnostics ---")
        # --- 结束诊断 ---

        message(STATUS "Qt6 found - Enabling Qt support")
        target_sources(json_struct PRIVATE
            src/qt_types/qt_ultimate_registry.cpp
            src/qt_types/qt_common.cpp
        )
        # 显式添加 Qt 的包含目录，以增强兼容性
        target_include_directories(json_struct PRIVATE
             ${CMAKE_CURRENT_SOURCE_DIR}/src/qt_types
             ${Qt6_INCLUDE_DIRS}
        )
        target_link_libraries(json_struct PUBLIC Qt6::Core Qt6::Widgets)
    else()
        message(WARNING "Qt support was enabled, but Qt6 was not found.")
    endif()
endif()

# -----------------------------------------------------------------------------
# V. Subdirectories (Tests & Examples)
# -----------------------------------------------------------------------------

if(BUILD_TESTS)
    enable_testing() # 启用 CTest
    add_subdirectory(test_framework)
    add_subdirectory(tests)
endif()

# -----------------------------------------------------------------------------
# VII. Fuzzing Support (Optional)
# -----------------------------------------------------------------------------
if(BUILD_FUZZER)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(WARNING "Fuzzing requires Clang compiler. Current compiler: ${CMAKE_CXX_COMPILER_ID}. Disabling BUILD_FUZZER.")
        set(BUILD_FUZZER OFF CACHE BOOL "Build fuzzing tests with libFuzzer (requires Clang)" FORCE)
    else()
        message(STATUS "Fuzzing enabled with Clang compiler.")
        add_subdirectory(tests/fuzzing)
    endif()
endif()

# -----------------------------------------------------------------------------
# VI. Installation Rules
# -----------------------------------------------------------------------------
include(GNUInstallDirs) # 引入标准安装路径变量 (CMAKE_INSTALL_LIBDIR, etc.)
include(CMakePackageConfigHelpers)

# 1. 安装目标及其 "Targets" 导出文件
install(TARGETS json_struct
    EXPORT json_structTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# 2. 安装公共头文件
install(DIRECTORY src/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/json_struct
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*/internal" EXCLUDE
)
#    单独安装生成的版本头文件
install(FILES ${GENERATED_INCLUDE_DIR}/version.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/json_struct
)

# 3. 生成并安装版本文件 (ConfigVersion.cmake)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/json_structConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# 4. 生成并安装配置文件 (Config.cmake)
configure_file("json_structConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/json_structConfig.cmake"
    @ONLY
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/json_structConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/json_structConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/json_struct
)

# 5. 安装导出的 Targets 文件
install(EXPORT json_structTargets
    FILE json_structTargets.cmake
    NAMESPACE json_struct::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/json_struct
)

# =============================================================================
#                         End of JsonStruct Project
# =============================================================================
