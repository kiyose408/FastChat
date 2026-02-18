$env:PATH = "D:\Qt\6.9.0\mingw_64\bin;D:\Qt\Tools\CMake_64\bin;D:\Qt\Tools\mingw1310_64\bin;$env:PATH"

Write-Host "=== Building (without clean) ===" -ForegroundColor Yellow
Set-Location "d:\Git\fastchat\build\Release"

cmake --build . --config Release

if ($LASTEXITCODE -eq 0) {
    Write-Host "=== Build Complete! ===" -ForegroundColor Green
} else {
    Write-Host "=== Build Failed! ===" -ForegroundColor Red
}
