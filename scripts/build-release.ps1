# FastChat v0.1.0-beta Build Script
# Usage: Run this script in PowerShell

param(
    [string]$QtDir = "D:\Qt\6.9.0\mingw_64",
    [string]$CMakeDir = "D:\Qt\Tools\CMake_64\bin",
    [string]$NinjaDir = "D:\Qt\Tools\Ninja",
    [string]$MingwDir = "D:\Qt\Tools\mingw1310_64\bin",
    [string]$BuildDir = "d:\Git\fastchat\build\Release",
    [string]$SourceDir = "d:\Git\fastchat",
    [string]$OutputDir = "d:\Git\fastchat\dist\FastChat-0.1.0-beta"
)

Write-Host "=== FastChat v0.1.0-beta Build Script ===" -ForegroundColor Green

# Set environment variables
$env:PATH = "$QtDir\bin;$CMakeDir;$NinjaDir;$MingwDir;$env:PATH"

Write-Host "Qt Dir: $QtDir"
Write-Host "CMake Dir: $CMakeDir"
Write-Host "Mingw Dir: $MingwDir"

# Create build directory
Write-Host "1. Creating build directory..." -ForegroundColor Yellow
if (Test-Path $BuildDir) {
    Remove-Item -Recurse -Force $BuildDir
}
New-Item -ItemType Directory -Path $BuildDir | Out-Null

# CMake configuration
Write-Host "2. Configuring CMake..." -ForegroundColor Yellow
Set-Location $BuildDir
& cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$QtDir" "$SourceDir"

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    exit 1
}

# Build
Write-Host "3. Building project..." -ForegroundColor Yellow
& cmake --build . --config Release

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

# Create release directory
Write-Host "4. Creating release directory..." -ForegroundColor Yellow
if (Test-Path $OutputDir) {
    Remove-Item -Recurse -Force $OutputDir
}
New-Item -ItemType Directory -Path $OutputDir | Out-Null

# Copy executable
Write-Host "5. Copying executable..." -ForegroundColor Yellow
Copy-Item "$BuildDir\fastchat.exe" $OutputDir

# Use windeployqt to collect dependencies
Write-Host "6. Collecting Qt dependencies..." -ForegroundColor Yellow
Set-Location $OutputDir
& "$QtDir\bin\windeployqt.exe" --release fastchat.exe

# Create config directory
Write-Host "7. Creating config directory..." -ForegroundColor Yellow
New-Item -ItemType Directory -Path "$OutputDir\config" | Out-Null

# Create README
Write-Host "8. Creating README..." -ForegroundColor Yellow
$readmeContent = @"
FastChat v0.1.0-beta
====================

A Qt-based instant messaging application.

Features:
- User registration/login
- Friend management (add, delete, notes)
- Real-time messaging (WebSocket)
- Image/file sending

Usage:
1. Make sure backend service is running (default: http://localhost:3000)
2. Run fastchat.exe
3. Register a new account or login with existing one

Requirements:
- Windows 10/11
- Network connection

Version: 0.1.0-beta
Release Date: $(Get-Date -Format "yyyy-MM-dd")
"@
Set-Content -Path "$OutputDir\README.txt" -Value $readmeContent -Encoding UTF8

Write-Host ""
Write-Host "=== Build Complete! ===" -ForegroundColor Green
Write-Host "Output directory: $OutputDir" -ForegroundColor Cyan
Write-Host ""
Write-Host "Directory contents:" -ForegroundColor Yellow
Get-ChildItem $OutputDir | Format-Table Name, Length, LastWriteTime
