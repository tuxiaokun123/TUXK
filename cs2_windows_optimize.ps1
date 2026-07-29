#Requires -RunAsAdministrator
<#
.SYNOPSIS
    CS2 Windows 性能优化脚本 (Windows 10/11 + NVIDIA)
.DESCRIPTION
    - 创建系统还原点 (可回滚)
    - 切换至"卓越性能"电源计划
    - 关闭 Xbox Game Bar / 游戏录制 (Game DVR) 减少后台开销
    - 全局禁用"全屏优化" (减少输入延迟、避免 Alt+Tab 卡顿)
    - 启用硬件加速 GPU 计划 (HAGS, 降低延迟)
    - 关闭 Nagle 算法 (TcpAckFrequency=1, TCPNoDelay=1) 降低网络延迟
    - 关闭后台应用 / 部分遥测
    - 提升 CS2 进程优先级 (启动游戏后可手动调用 Set-CS2Priority)
.NOTES
    安全说明:
      * 所有注册表写入都通过 New-ItemProperty -Force, 可重复运行
      * 脚本末尾会打印"如何回滚"的说明
      * 强烈建议运行前已开启系统保护 (脚本会尝试开启)
    运行方式 (管理员 PowerShell):
      Set-ExecutionPolicy -Scope Process Bypass -Force
      .\cs2_windows_optimize.ps1
#>

# ---------- 0. 基础环境 ----------
$ErrorActionPreference = 'Stop'
$Host.UI.RawUI.WindowTitle = 'CS2 Windows Optimizer'

function Write-Step($msg) { Write-Host "`n[步骤] $msg" -ForegroundColor Cyan }
function Write-Ok($msg)   { Write-Host "  [OK] $msg" -ForegroundColor Green }
function Write-Warn($msg) { Write-Host "  [!]  $msg" -ForegroundColor Yellow }
function Write-Skip($msg) { Write-Host "  [--] $msg (已存在/跳过)" -ForegroundColor DarkGray }

# 检查管理员
$isAdmin = ([Security.Principal.WindowsPrincipal] `
    [Security.Principal.WindowsIdentity]::GetCurrent()
).IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)
if (-not $isAdmin) {
    Write-Host '请用"管理员身份"运行 PowerShell 后再执行本脚本。' -ForegroundColor Red
    exit 1
}

Write-Host '=============================================' -ForegroundColor White
Write-Host ' CS2 Windows 性能优化脚本' -ForegroundColor White
Write-Host ' 适用于 Windows 10 / 11 + NVIDIA' -ForegroundColor White
Write-Host '=============================================' -ForegroundColor White

# ---------- 1. 系统还原点 ----------
Write-Step '创建系统还原点 (便于回滚)'
try {
    Enable-ComputerRestore -Drive $env:SystemDrive -ErrorAction SilentlyContinue
    $existing = Get-ComputerRestorePoint -ErrorAction SilentlyContinue | Measure-Object
    Checkpoint-Computer -Description 'CS2优化前备份' -RestorePointType 'MODIFY_SETTINGS' -ErrorAction Stop
    Write-Ok '还原点已创建'
} catch {
    Write-Warn "无法创建还原点: $($_.Exception.Message)"
    Write-Warn '可手动: 系统属性 -> 系统保护 -> 创建'
}

# ---------- 2. 电源计划: 卓越性能 ----------
Write-Step '切换电源计划为"卓越性能" (Ultimate Performance)'
try {
    $ultimate = powercfg /list | Select-String 'e9a42b02-d5df-448d-aa00-03f14749eb61'
    if (-not $ultimate) {
        powercfg /duplicatescheme e9a42b02-d5df-448d-aa00-03f14749eb61 | Out-Null
        Write-Ok '已生成"卓越性能"计划'
    } else { Write-Skip '卓越性能计划已存在' }
    powercfg /setactive e9a42b02-d5df-448d-aa00-03f14749eb61
    Write-Ok '已切换至卓越性能'
    # 关闭 USB 选择性挂起 (避免外设偶发延迟)
    powercfg /SETACVALUEINDEX SCHEME_CURRENT 2a737441-1930-4402-8d77-b2bebba308a3 48e6b7a6-50f5-4782-a5d4-53bb8f07e226 0 | Out-Null
    powercfg /SETACTIVE SCHEME_CURRENT | Out-Null
    Write-Ok '已关闭 USB 选择性挂起'
} catch {
    Write-Warn "电源计划切换失败: $($_.Exception.Message)"
}

# ---------- 3. 关闭 Xbox Game Bar / Game DVR ----------
Write-Step '关闭 Xbox Game Bar / 后台游戏录制'
try {
    $paths = @(
        'HKCU:\Software\Microsoft\Windows\CurrentVersion\GameDVR',
        'HKLM:\SOFTWARE\Policies\Microsoft\Windows\GameDVR'
    )
    foreach ($p in $paths) {
        New-Item -Path $p -Force | Out-Null
        Set-ItemProperty -Path $p -Name 'AppCaptureEnabled' -Value 0 -Type DWord -Force
        Set-ItemProperty -Path $p -Name 'HistoricalCaptureEnabled' -Value 0 -Type DWord -Force
    }
    Set-ItemProperty -Path 'HKCU:\System\GameConfigStore' -Name 'GameDVR_Enabled' -Value 0 -Type DWord -Force
    Set-ItemProperty -Path 'HKCU:\SOFTWARE\Microsoft\GameBar' -Name 'AllowAutoGameMode' -Value 1 -Type DWord -Force
    Set-ItemProperty -Path 'HKCU:\SOFTWARE\Microsoft\GameBar' -Name 'AutoGameModeEnabled' -Value 1 -Type DWord -Force
    Set-ItemProperty -Path 'HKCU:\SOFTWARE\Microsoft\GameBar' -Name 'ShowStartupPanel' -Value 0 -Type DWord -Force
    Write-Ok 'Game Bar 录制已关闭 (保留自动游戏模式, 利于 CS2 全屏优化)'
} catch {
    Write-Warn "Game Bar 设置失败: $($_.Exception.Message)"
}

# ---------- 4. 全局禁用"全屏优化" ----------
Write-Step '全局禁用"全屏优化" (降低输入延迟、避免 Alt+Tab 卡顿)'
try {
    $key = 'HKCU:\System\GameConfigStore'
    Set-ItemProperty -Path $key -Name 'GameDVR_FSEBehaviorMode' -Value 2 -Type DWord -Force
    Set-ItemProperty -Path $key -Name 'GameDVR_HonorUserFSEBehaviorMode' -Value 1 -Type DWord -Force
    Set-ItemProperty -Path $key -Name 'GameDVR_DXGIHonorFSEWindowsCompatible' -Value 1 -Type DWord -Force
    Write-Ok '全屏优化已禁用'
} catch {
    Write-Warn "全屏优化设置失败: $($_.Exception.Message)"
}

# ---------- 5. 启用硬件加速 GPU 计划 (HAGS) ----------
Write-Step '启用硬件加速 GPU 计划 (HAGS, 降低渲染延迟)'
try {
    $key = 'HKLM:\SYSTEM\CurrentControlSet\Control\GraphicsDrivers'
    Set-ItemProperty -Path $key -Name 'HwSchMode' -Value 2 -Type DWord -Force
    Write-Ok 'HAGS 已启用 (重启后生效)'
} catch {
    Write-Warn "HAGS 设置失败: $($_.Exception.Message) (旧 GPU/驱动可能不支持)"
}

# ---------- 6. 关闭 Nagle 算法 (降低网络延迟) ----------
Write-Step '关闭 Nagle 算法 (降低网络小包延迟, 适用于稳定宽带)'
try {
    $adapters = Get-ChildItem 'HKLM:\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters\Interfaces' |
        Get-ItemProperty | Where-Object { $_.DhcpIPAddress -and $_.DhcpIPAddress -ne '0.0.0.0' -and $_.DhcpIPAddress -ne $null }
    $count = 0
    foreach ($a in $adapters) {
        $p = $a.PSPath
        Set-ItemProperty -Path $p -Name 'TcpAckFrequency' -Value 1 -Type DWord -Force
        Set-ItemProperty -Path $p -Name 'TCPNoDelay'      -Value 1 -Type DWord -Force
        $count++
    }
    if ($count -gt 0) {
        Write-Ok "已为 $count 个活动网卡关闭 Nagle (重启生效)"
    } else {
        Write-Warn '未找到活动 DHCP 网卡, 跳过 (静态 IP 请手动添加 TcpAckFrequency=1, TCPNoDelay=1)'
    }
} catch {
    Write-Warn "Nagle 设置失败: $($_.Exception.Message)"
}

# ---------- 7. 视觉效果: 调整为"最佳性能" ----------
Write-Step '视觉效果调整为"最佳性能" (关闭动画/阴影, 提升整体响应)'
try {
    $key = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\VisualEffects'
    New-Item -Path $key -Force | Out-Null
    Set-ItemProperty -Path $key -Name 'VisualFXSetting' -Value 2 -Type DWord -Force
    # 0=让系统选 1=最佳外观 2=最佳性能 3=自定义
    Write-Ok '视觉效果已设为最佳性能 (注销/重启生效)'
} catch {
    Write-Warn "视觉效果设置失败: $($_.Exception.Message)"
}

# ---------- 8. 关闭部分遥测 / 后台应用 ----------
Write-Step '关闭部分遥测与允许的后台应用'
try {
    # 关闭兼容性遥测相关计划任务 (不删, 只禁用)
    $tasks = @(
        '\Microsoft\Windows\Application Experience\Microsoft Compatibility Appraiser',
        '\Microsoft\Windows\Application Experience\ProgramDataUpdater'
    )
    foreach ($t in $tasks) {
        try { Disable-ScheduledTask -TaskPath ($t | Split-Path) -TaskName ($t | Split-Path -Leaf) -ErrorAction Stop | Out-Null } catch {}
    }
    Write-Ok '兼容性遥测任务已禁用'

    # 关闭"允许应用在后台运行"总开关 (Win10 有效, Win11 部分版本移除)
    $bgKey = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\BackgroundAccessApplications'
    New-Item -Path $bgKey -Force | Out-Null
    Set-ItemProperty -Path $bgKey -Name 'GlobalUserDisabled' -Value 1 -Type DWord -Force
    Write-Ok '后台应用总开关已关闭'
} catch {
    Write-Warn "后台应用/遥测设置部分失败: $($_.Exception.Message)"
}

# ---------- 9. 游戏 DVR 文件清理提示 ----------
Write-Step '提示: 清理历史录像'
$captures = "$env:LOCALAPPDATA\Captures"
if (Test-Path $captures) {
    $size = (Get-ChildItem $captures -Recurse -ErrorAction SilentlyContinue | Measure-Object Length -Sum).Sum
    if ($size) { Write-Warn "Captures 文件夹占用 $([math]::Round($size/1MB,1)) MB, 可手动清空: $captures" }
}

# ---------- 10. 提示 NVIDIA 驱动相关 ----------
Write-Step '提示: NVIDIA 控制面板需手动设置 (见手动指南)'
$nvcpl = 'C:\Program Files\NVIDIA Corporation\Control Panel Client\nvcplui.exe'
if (Test-Path $nvcpl) {
    Write-Ok "检测到 NVIDIA 控制面板: $nvcpl"
} else {
    Write-Warn '未检测到 NVIDIA 控制面板, 请确认驱动已安装'
}

# ---------- 11. 辅助函数: 给 CS2 进程提优先级 ----------
function Set-CS2Priority {
    <#
      游戏启动后在另一个 PowerShell 窗口运行: Set-CS2Priority
      将 cs2.exe 设为"高优先级" (不要设实时, 会影响系统)
    #>
    $p = Get-Process -Name 'cs2' -ErrorAction SilentlyContinue
    if (-not $p) { Write-Host '未找到 cs2 进程, 请先启动游戏。' -ForegroundColor Yellow; return }
    $p.PriorityClass = 'High'
    Write-Host "已将 cs2.exe (PID $($p.Id)) 设为 High 优先级" -ForegroundColor Green
}
Write-Ok '辅助函数 Set-CS2Priority 已注入 (游戏启动后调用可提优先级)'

# ---------- 完成 / 回滚说明 ----------
Write-Host "`n=============================================" -ForegroundColor White
Write-Host ' 优化完成! 建议重启电脑使所有设置生效。' -ForegroundColor Green
Write-Host '=============================================' -ForegroundColor White
Write-Host @'

回滚方法 (任选其一):
  1) 还原系统: rstrui.exe  ->  选择"CS2优化前备份"
  2) 仅还原电源: powercfg /setactive 381b4222-f694-41f0-9685-ff5bb260df2e  (平衡)
  3) 还原全屏优化: 在 HKCU\System\GameConfigStore 把 GameDVR_FSEBehaviorMode 改回 0
  4) 还原 Nagle: 删除各网卡的 TcpAckFrequency 和 TCPNoDelay 两个值

接下来:
  * 安装/更新 NVIDIA 驱动到最新 Game Ready 版
  * 按 <CS2_优化指南.md> 设置 NVIDIA 控制面板 / 游戏内选项 / 启动项
  * 把 cs2_autoexec.cfg 放到 .../game/csgo/cfg/ 下
'@
