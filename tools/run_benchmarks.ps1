#!/usr/bin/env pwsh

# Configuration
$BenchmarkMinTime = "0.01s"
$BenchmarkDir = Join-Path $PSScriptRoot ".." "build" "core" "memory" "Release"

# Get all benchmark executables
$BenchmarkFiles = Get-ChildItem -Path $BenchmarkDir -Filter "*benchmark*.exe"

foreach ($Benchmark in $BenchmarkFiles) {
    Write-Host "`nRunning benchmark: $($Benchmark.Name)"
    Write-Host "====================================`n"
    & $Benchmark.FullName --benchmark_min_time=$BenchmarkMinTime
}