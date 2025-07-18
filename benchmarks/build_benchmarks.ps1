# JsonStruct 性能基准测试独立构建脚本
# 此脚本用于独立构建和运行性能对比测试，与主项目完全分离

param(
    [string]$BuildType = "Release",
    [string]$Generator = "Visual Studio 17 2022",
    [switch]$WithRealLibraries = $true,
    [switch]$WithQt = $false,
    [switch]$Clean = $false,
    [switch]$RunTests = $false,
    [switch]$Help = $false
)

# 显示帮助信息
if ($Help) {
    Write-Host "JsonStruct 性能基准测试独立构建脚本" -ForegroundColor Green
    Write-Host ""
    Write-Host "参数说明:" -ForegroundColor Yellow
    Write-Host "  -BuildType <type>      构建类型 (Release, Debug, RelWithDebInfo)" -ForegroundColor White
    Write-Host "  -Generator <gen>       CMake生成器 (默认: Visual Studio 17 2022)" -ForegroundColor White
    Write-Host "  -WithRealLibraries     启用真实JSON库对比测试 (默认: true)" -ForegroundColor White
    Write-Host "  -WithQt               启用Qt支持 (默认: false)" -ForegroundColor White
    Write-Host "  -Clean                清理构建目录" -ForegroundColor White
    Write-Host "  -RunTests             构建完成后运行测试" -ForegroundColor White
    Write-Host "  -Help                 显示此帮助信息" -ForegroundColor White
    Write-Host ""
    Write-Host "使用示例:" -ForegroundColor Yellow
    Write-Host "  .\build_benchmarks.ps1                        # 默认构建" -ForegroundColor White
    Write-Host "  .\build_benchmarks.ps1 -BuildType Debug       # Debug构建" -ForegroundColor White
    Write-Host "  .\build_benchmarks.ps1 -Clean                 # 清理构建" -ForegroundColor White
    Write-Host "  .\build_benchmarks.ps1 -RunTests              # 构建并运行测试" -ForegroundColor White
    Write-Host "  .\build_benchmarks.ps1 -WithRealLibraries:$false # 不使用真实JSON库" -ForegroundColor White
    exit 0
}

# 设置工作目录
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BenchmarkDir = $ScriptDir
$BuildDir = Join-Path $BenchmarkDir "build"

Write-Host "===========================================" -ForegroundColor Green
Write-Host "JsonStruct 性能基准测试独立构建" -ForegroundColor Green
Write-Host "===========================================" -ForegroundColor Green
Write-Host "构建类型: $BuildType" -ForegroundColor Yellow
Write-Host "生成器: $Generator" -ForegroundColor Yellow
Write-Host "真实JSON库: $WithRealLibraries" -ForegroundColor Yellow
Write-Host "Qt支持: $WithQt" -ForegroundColor Yellow
Write-Host "工作目录: $BenchmarkDir" -ForegroundColor Yellow
Write-Host "构建目录: $BuildDir" -ForegroundColor Yellow
Write-Host "===========================================" -ForegroundColor Green

# 清理构建目录
if ($Clean) {
    Write-Host "清理构建目录..." -ForegroundColor Blue
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir
        Write-Host "构建目录已清理" -ForegroundColor Green
    }
}

# 创建构建目录
if (-not (Test-Path $BuildDir)) {
    Write-Host "创建构建目录..." -ForegroundColor Blue
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# 进入构建目录
Set-Location $BuildDir

try {
    # 构建CMake配置命令
    $CMakeArgs = @(
        "-G", $Generator,
        "-DCMAKE_BUILD_TYPE=$BuildType",
        "-DBUILD_REAL_JSON_COMPARISON=$($WithRealLibraries.ToString().ToLower())",
        "-DENABLE_QT_SUPPORT_BENCHMARK=$($WithQt.ToString().ToLower())",
        "-S", $BenchmarkDir,
        "-B", $BuildDir
    )

    Write-Host "配置项目..." -ForegroundColor Blue
    Write-Host "CMake命令: cmake $($CMakeArgs -join ' ')" -ForegroundColor Gray
    
    $ConfigureProcess = Start-Process -FilePath "cmake" -ArgumentList $CMakeArgs -NoNewWindow -Wait -PassThru
    
    if ($ConfigureProcess.ExitCode -ne 0) {
        Write-Host "CMake配置失败！" -ForegroundColor Red
        exit 1
    }
    
    Write-Host "项目配置成功！" -ForegroundColor Green

    # 构建项目
    Write-Host "构建项目..." -ForegroundColor Blue
    $BuildArgs = @(
        "--build", $BuildDir,
        "--config", $BuildType,
        "--target", "build_all_benchmarks"
    )
    
    Write-Host "构建命令: cmake $($BuildArgs -join ' ')" -ForegroundColor Gray
    
    $BuildProcess = Start-Process -FilePath "cmake" -ArgumentList $BuildArgs -NoNewWindow -Wait -PassThru
    
    if ($BuildProcess.ExitCode -ne 0) {
        Write-Host "项目构建失败！" -ForegroundColor Red
        exit 1
    }
    
    Write-Host "项目构建成功！" -ForegroundColor Green

    # 列出生成的可执行文件
    Write-Host "生成的可执行文件:" -ForegroundColor Blue
    $ExeFiles = Get-ChildItem -Path $BuildDir -Recurse -Filter "*.exe" | Where-Object { $_.Name -match "benchmark|performance|memory|reporter" }
    foreach ($exe in $ExeFiles) {
        Write-Host "  $($exe.FullName)" -ForegroundColor White
    }

    # 运行测试
    if ($RunTests) {
        Write-Host "运行性能测试..." -ForegroundColor Blue
        
        # 找到可执行文件目录
        $ExeDir = $BuildDir
        if ($BuildType -eq "Release" -or $BuildType -eq "Debug") {
            $PossibleExeDir = Join-Path $BuildDir $BuildType
            if (Test-Path $PossibleExeDir) {
                $ExeDir = $PossibleExeDir
            }
        }
        
        Write-Host "可执行文件目录: $ExeDir" -ForegroundColor Gray
        
        # 运行各种测试
        $TestExecutables = @(
            "performance_benchmark.exe",
            "memory_benchmark.exe", 
            "comparison_benchmark_simple.exe",
            "performance_reporter.exe"
        )
        
        foreach ($testExe in $TestExecutables) {
            $testPath = Join-Path $ExeDir $testExe
            if (Test-Path $testPath) {
                Write-Host "运行 $testExe..." -ForegroundColor Yellow
                try {
                    & $testPath
                    Write-Host "$testExe 运行完成" -ForegroundColor Green
                } catch {
                    Write-Host "$testExe 运行失败: $_" -ForegroundColor Red
                }
                Write-Host "----------------------------------------" -ForegroundColor Gray
            } else {
                Write-Host "未找到 $testExe" -ForegroundColor Orange
            }
        }
        
        # 如果启用了真实库对比，运行对比测试
        if ($WithRealLibraries) {
            $comparisonPath = Join-Path $ExeDir "comparison_benchmark.exe"
            if (Test-Path $comparisonPath) {
                Write-Host "运行真实库对比测试..." -ForegroundColor Yellow
                try {
                    & $comparisonPath
                    Write-Host "真实库对比测试完成" -ForegroundColor Green
                } catch {
                    Write-Host "真实库对比测试失败: $_" -ForegroundColor Red
                }
            }
        }
    }

    Write-Host "===========================================" -ForegroundColor Green
    Write-Host "构建完成！" -ForegroundColor Green
    Write-Host "===========================================" -ForegroundColor Green
    
    if (-not $RunTests) {
        Write-Host "要运行测试，请使用: .\build_benchmarks.ps1 -RunTests" -ForegroundColor Yellow
        Write-Host "或者手动运行构建目录中的可执行文件" -ForegroundColor Yellow
    }

} catch {
    Write-Host "构建过程中发生错误: $_" -ForegroundColor Red
    exit 1
} finally {
    # 回到原始目录
    Set-Location $ScriptDir
}
