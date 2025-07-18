# 快速运行基准测试脚本
# 用于快速运行已构建的基准测试

param(
    [string]$BuildType = "Release",
    [switch]$All = $false,
    [switch]$Performance = $false,
    [switch]$Memory = $false,
    [switch]$Comparison = $false,
    [switch]$Simple = $false,
    [switch]$Help = $false
)

if ($Help) {
    Write-Host "JsonStruct 基准测试运行脚本" -ForegroundColor Green
    Write-Host ""
    Write-Host "参数说明:" -ForegroundColor Yellow
    Write-Host "  -BuildType <type>  构建类型 (Release, Debug)" -ForegroundColor White
    Write-Host "  -All              运行所有测试" -ForegroundColor White
    Write-Host "  -Performance      运行性能测试" -ForegroundColor White
    Write-Host "  -Memory           运行内存测试" -ForegroundColor White
    Write-Host "  -Comparison       运行对比测试" -ForegroundColor White
    Write-Host "  -Simple           运行简单对比测试" -ForegroundColor White
    Write-Host "  -Help             显示帮助信息" -ForegroundColor White
    Write-Host ""
    Write-Host "使用示例:" -ForegroundColor Yellow
    Write-Host "  .\run_benchmarks.ps1 -All         # 运行所有测试" -ForegroundColor White
    Write-Host "  .\run_benchmarks.ps1 -Performance # 仅运行性能测试" -ForegroundColor White
    Write-Host "  .\run_benchmarks.ps1 -Simple      # 运行简单对比测试" -ForegroundColor White
    exit 0
}

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir = Join-Path $ScriptDir "build"

# 确定可执行文件目录
$ExeDir = $BuildDir
if (Test-Path (Join-Path $BuildDir $BuildType)) {
    $ExeDir = Join-Path $BuildDir $BuildType
}

if (-not (Test-Path $ExeDir)) {
    Write-Host "构建目录不存在: $ExeDir" -ForegroundColor Red
    Write-Host "请先运行 .\build_benchmarks.ps1 构建项目" -ForegroundColor Yellow
    exit 1
}

Write-Host "JsonStruct 基准测试运行器" -ForegroundColor Green
Write-Host "可执行文件目录: $ExeDir" -ForegroundColor Yellow
Write-Host "===========================================" -ForegroundColor Green

# 定义测试映射
$Tests = @{
    "Performance" = "performance_benchmark.exe"
    "Memory" = "memory_benchmark.exe"
    "Comparison" = "comparison_benchmark.exe"
    "Simple" = "comparison_benchmark_simple.exe"
    "Reporter" = "performance_reporter.exe"
}

# 运行指定的测试
function Run-Test {
    param($TestName, $ExePath)
    
    $FullPath = Join-Path $ExeDir $ExePath
    if (Test-Path $FullPath) {
        Write-Host "运行 $TestName 测试..." -ForegroundColor Blue
        try {
            $StartTime = Get-Date
            & $FullPath
            $EndTime = Get-Date
            $Duration = $EndTime - $StartTime
            Write-Host "$TestName 测试完成 (耗时: $($Duration.TotalSeconds.ToString('F2'))s)" -ForegroundColor Green
        } catch {
            Write-Host "$TestName 测试失败: $_" -ForegroundColor Red
        }
    } else {
        Write-Host "$TestName 测试程序不存在: $FullPath" -ForegroundColor Red
    }
    Write-Host "-------------------------------------------" -ForegroundColor Gray
}

# 根据参数运行测试
if ($All) {
    foreach ($test in $Tests.GetEnumerator()) {
        Run-Test $test.Key $test.Value
    }
} else {
    if ($Performance) { Run-Test "Performance" $Tests["Performance"] }
    if ($Memory) { Run-Test "Memory" $Tests["Memory"] }
    if ($Comparison) { Run-Test "Comparison" $Tests["Comparison"] }
    if ($Simple) { Run-Test "Simple" $Tests["Simple"] }
    
    # 如果没有指定任何测试，运行默认测试
    if (-not ($Performance -or $Memory -or $Comparison -or $Simple)) {
        Write-Host "未指定测试类型，运行默认测试..." -ForegroundColor Yellow
        Run-Test "Performance" $Tests["Performance"]
        Run-Test "Simple" $Tests["Simple"]
        Run-Test "Reporter" $Tests["Reporter"]
    }
}

Write-Host "===========================================" -ForegroundColor Green
Write-Host "所有测试完成！" -ForegroundColor Green
