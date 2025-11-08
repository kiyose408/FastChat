$ExcludeFolders = @('.git', '.qtcreator', 'build')

function Get-TreeASCII {
    param(
        [string]$Path = ".",
        [string]$Prefix = ""
    )

    # 获取当前路径下的所有子项（文件夹+文件）
    $items = Get-ChildItem -Path $Path -Force -ErrorAction SilentlyContinue

    # 过滤：排除指定文件夹，保留所有文件
    $filtered = @()
    foreach ($item in $items) {
        if ($item.PSIsContainer) {
            if ($item.Name -notin $ExcludeFolders) {
                $filtered += $item
            }
        } else {
            $filtered += $item
        }
    }

    # 按文件夹优先排序（可选，但更清晰）
    $filtered = $filtered | Sort-Object @{Expression={$_.PSIsContainer}; Descending=$true}

    $total = $filtered.Count
    for ($i = 0; $i -lt $total; $i++) {
        $item = $filtered[$i]
        $isLast = ($i -eq $total - 1)

        # ✅ 修正1：使用 $(if ...) 而不是 if ... 直接赋值
        $symbol = $(if ($isLast) { "`-- " } else { "+-- " })
        Write-Output "${Prefix}${symbol}$($item.Name)"

        # 如果是未被排除的文件夹，递归处理其内容
        if ($item.PSIsContainer) {
            # ✅ 修正2：使用 $(if ...) 包裹条件表达式
            $nextPrefix = $Prefix + $(if ($isLast) { "    " } else { "|   " })
            Get-TreeASCII -Path $item.FullName -Prefix $nextPrefix
        }
    }
}

# 获取当前目录名作为根
$rootName = (Get-Item .).Name
$outputLines = @($rootName)
$outputLines += Get-TreeASCII

# 输出到文件（ASCII 字符，用 Default 编码即可）
Set-Content -Path "flodtree.txt" -Value $outputLines -Encoding Default

# ========== 新增：暂停等待用户按键 ==========
Write-Host "`ngenrate-tree.ps1 run over, Click Enter to Exit..." -ForegroundColor Green
Read-Host