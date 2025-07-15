# PowerShell script to run all test executables in build/Debug
echo "Running all JsonStruct test executables..."
$testDir = "build/Desktop_Qt_6_2_2_MSVC2019_64bit-Debug"
$testPattern = "test_*.exe"
$testFiles = Get-ChildItem -Path $testDir -Filter $testPattern | Sort-Object Name

$failures = 0
foreach ($test in $testFiles) {
    Write-Host "\n=== Running $($test.Name) ==="
    & "$($test.FullName)"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[FAILED] $($test.Name) exited with code $LASTEXITCODE" -ForegroundColor Red
        $failures++
    } else {
        Write-Host "[PASSED] $($test.Name)"
    }
}

if ($failures -eq 0) {
    Write-Host "\nAll tests passed!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "\nSome tests failed: $failures" -ForegroundColor Red
    exit 1
}
