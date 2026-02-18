$env:PATH = "D:\Qt\6.9.0\mingw_64\bin;D:\Qt\Tools\CMake_64\bin;D:\Qt\Tools\mingw1310_64\bin;$env:PATH"

Write-Host "=== Cleaning Build Directory ==="
if (Test-Path "d:\Git\fastchat\build\Release") {
    Remove-Item -Recurse -Force "d:\Git\fastchat\build\Release"
}
New-Item -ItemType Directory -Path "d:\Git\fastchat\build\Release" -Force | Out-Null

Write-Host "=== CMake Configure ==="
Set-Location "d:\Git\fastchat\build\Release"

cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="D:\Qt\6.9.0\mingw_64" "d:\Git\fastchat"

if ($LASTEXITCODE -eq 0) {
    Write-Host "=== Building ==="
    cmake --build . --config Release
}
