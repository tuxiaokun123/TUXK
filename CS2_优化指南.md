# CS2 优化指南 (Windows 10/11 + NVIDIA)

> 目标: 最高 FPS、低输入延迟、画面流畅平衡、稳定性。配套文件:
> - `cs2_autoexec.cfg` — 游戏内配置
> - `cs2_windows_optimize.ps1` — 系统一键优化脚本
> - 本文档 — 需手动设置的部分 (驱动面板 / 启动项 / 游戏内选项)

按顺序执行下面 7 个章节即可。

---

## 0. 准备: 跑系统优化脚本 (强烈建议先做)

1. **以管理员身份**打开 PowerShell
2. 执行:
   ```powershell
   Set-ExecutionPolicy -Scope Process Bypass -Force
   # 把脚本路径换成你存放的位置
   & "D:\你的路径\cs2_windows_optimize.ps1"
   ```
3. 脚本会自动: 建还原点 → 卓越性能电源 → 关 Game DVR → 关全屏优化 → 开 HAGS → 关 Nagle → 关背景应用
4. **重启电脑**使 HAGS / Nagle / 电源计划生效

> 脚本结束会打印回滚方法, 不放心可随时还原。

---

## 1. NVIDIA 驱动

1. 到 [nvidia.com/drivers](https://www.nvidia.com/Download/index.aspx) 下载 **Game Ready 驱动** (Studio 版也行, 但 GRD 对游戏更新更及时)
2. 安装时选 **自定义 → 勾选"执行清洁安装"**
3. 安装完打开 **NVIDIA 控制面板** (桌面右键)

### 1.1 全局 3D 设置 (管理 3D 设置 → 全局设置)

| 选项 | 推荐值 | 原因 |
|---|---|---|
| 三重缓冲 | 关 | 仅 OpenGL, 关掉省资源 |
| 各向异性过滤 | 由程序控制 / 8x | CS2 内部有同样选项, 程序控制即可 |
| 垂直同步 | 关 | 消除输入延迟, FPS 解锁 |
| G-SYNC | 对窗口/全屏模式启用 (如显示器支持) | 平滑无撕裂; **配合游戏内 VSync 关闭 + Reflex Boost** |
| 低延迟模式 | **Ultra** | 等价预渲染帧 1, 降低输入延迟 (游戏内开了 Reflex 可置"开"或"Ultra"都行, 二者会协调) |
| 电源管理模式 | **首选最高性能** | 防止 GPU 降频掉帧 |
| 纹理过滤 - 质量 | **高性能** | 提升帧数, 画质差异极小 |
| 线程优化 | 开 | 多核渲染 |
| 着色器缓存大小 | 10 GB / 无限制 | 减少卡顿 |
| 最大帧速率 | 关 | 由游戏内 fps_max 控制 |
| OpenGL 渲染 GPU | 选你的主显卡 | 多显卡系统避免走错卡 |
| Vulkan/OpenGL 现行版本 | 优先最新 | 部分场景提升兼容 |

### 1.2 程序级 (为 cs2.exe 单独设)

1. "管理 3D 设置 → 程序设置 → 添加"
2. 路径: `Steam\steamapps\common\Counter-Strike Global Offensive\game\bin\win64\cs2.exe`
3. 至少把 **电源管理模式 = 首选最高性能**、**低延迟模式 = Ultra** 在程序级再确认一次

### 1.3 显示

- **更改分辨率**: 设到显示器原生分辨率 + 最高刷新率 (例 1920×1080@240Hz)
- **设置 G-SYNC**: 启用"适用于窗口和全屏模式"
- **使用 NVIDIA 颜色设置**: 输出动态范围选 **完全**, 颜色深度 8/10 bpc, 提升画面通透

---

## 2. Steam 启动项

Steam → 库 → 右键 CS2 → 属性 → 通用 → **启动选项**, 填入:

```
-novid -high -nojoy -fullscreen +fps_max 0 +exec autoexec.cfg
```

说明:
- `-novid` 跳过开场动画
- `-high` 进程高优先级 (与脚本里 `Set-CS2Priority` 二选一即可; 若系统调度异常可去掉)
- `-nojoy` 不加载手柄支持, 省一点资源
- `-fullscreen` 强制独占全屏 (配合脚本关全屏优化, 延迟最低)
- `+fps_max 0` 解锁帧率
- `+exec autoexec.cfg` 强制执行配置 (CS2 默认会执行, 加上更稳)

> **不要**再添加 `-threads`、`-tickrate`、`-d3d9ex` 等 CSGO 时代参数, CS2 已忽略或无效, 部分会引起问题。

---

## 3. 放置 autoexec.cfg

把 `cs2_autoexec.cfg` 复制到:

```
Steam\steamapps\common\Counter-Strike Global Offensive\game\csgo\cfg\autoexec.cfg
```

首次进游戏后, 在控制台输入 `exec autoexec` 手动跑一次确认无报错。之后每次启动自动加载。

> 想自定义准星/灵敏度/键位, 直接改这个文件对应行, 然后 `host_writeconfig` 即可。

---

## 4. 游戏内设置 (设置 → 视频)

### 4.1 显示

| 选项 | 推荐 | 备注 |
|---|---|---|
| 显示模式 | **全屏** | 独占全屏延迟最低 |
| 宽高比 | 4:3 拉伸 (竞技) 或 16:9 | 4:3 拉伸模型变宽易瞄准, 16:9 视野更广 |
| 分辨率 | 1280×960 (4:3) 或 原生 | 低分辨率大幅提帧; 高刷屏可选原生+高刷 |
| 刷新率 | 显示器最高 | |
| 亮度 | 100% ~ 110% | 暗处更易看人 |
| 颜色模式 | 电脑显示器 | |
| 玩家轮廓增强对比度 | **启用** | 显眼敌我, 几乎零性能成本 |
| 等待垂直同步 | **禁用** | 必关, 否则输入延迟 |
| NVIDIA Reflex 低延迟 | **启用 + Boost** | **必开**, 最低系统延迟 |

### 4.2 高级视频 (性能与画面平衡)

| 选项 | 推荐 (平衡) | 极限 FPS | 说明 |
|---|---|---|---|
| 增强玩家对比度 | 启用 | 启用 | 几乎零成本, 提升辨识 |
| 多采样抗锯齿模式 | 4x MSAA 或 CMAA2 | CMAA2 / 关 | MSAA 边缘更干净, CMAA2 更省 |
| 全局阴影质量 | 高 | 中/低 | 阴影影响看人, 建议高 |
| 模型/贴图细节 | 低 | 低 | 贴图细节对帧数影响小, 但竞技用低更清爽 |
| 纹理过滤 | 双线性/三线性 | 双线性 | 三线性略好, 双线性最快 |
| 着色器细节 | 低 | 低 | |
| 粒子细节 | 低 | 低 | 烟雾/爆炸效果降级, 提帧明显 |
| 环境光遮蔽 | **禁用** | 禁用 | 性能开销大, 竞技关掉 |
| 高动态范围 | **质量** | 性能 | |
| FidelityFX Super Resolution | **禁用 (最高画质)** | 性能 (如需补帧) | 竞技关闭最清晰; 低端机可用性能档提帧 |

> **NVIDIA Reflex 必开**, 它比任何 cvar 都更直接降低端到端延迟。

---

## 5. 鼠标输入延迟

- **关闭** Windows "提高指针精确度" (鼠标加速): 设置 → 蓝牙和其他设备 → 鼠标 → 取消勾选
- 鼠标回报率: 用厂商驱动设到 **1000 Hz** (高端鼠标可 2000/4000 Hz, 但 CPU 开销增大, 1000 Hz 已足够)
- CS2 内灵敏度 (`sensitivity`) 在游戏内设置, autoexec 里没写, 自行调整
- 用 **Raw Input**: CS2 默认开启原始输入, 无需额外设置

---

## 6. 网络 / 降低延迟

1. **优先有线**, 不要用 Wi-Fi 打竞技
2. 路由器开 **QoS**, 给游戏设备/CS2 端口优先
3. CS2 服务器端口: 主要走 UDP, 一般无需端口转发; 若 NAT 严格可开 UPnP
4. autoexec 里已设 `mm_dedicated_search_maxping 70`, 国内可按地区调 50~80
5. 关闭后台下载/网盘/视频/直播, 释放带宽
6. 如使用加速器: 选延迟最低节点, **避免**多开网页代理叠加

---

## 7. 稳定性 / 防卡顿防闪退

### 7.1 系统
- Windows 更新到最新, 装好芯片组驱动 (Intel INF / AMD CHIPSET, 尤其 AMD 要装 **Chipset Software** 含 PPM)
- 内存 ≥ 16GB; 关闭后台浏览器标签页 (Chrome 几十标签能吃 4GB+)
- 页面文件设为"系统管理", 不要禁用
- 关闭第三方杀毒的"游戏模式"覆盖, 避免与 Windows Defender 重复扫描; 把 CS2 安装目录加入排除
- 若 SSD 剩余空间 < 10%, 清理 — 着色器缓存需要空间

### 7.2 着色器缓存 (减少卡顿关键)
- NVIDIA 控制面板 → 管理 3D 设置 → **着色器缓存大小 = 10 GB**
- Windows → 设置 → 系统 → 存储 → 高级存储设置 → 确保保留足够空间
- CS2 首次进新地图会编译着色器, 前几局偶有卡顿属正常, 之后稳定

### 7.3 进程优先级
脚本已提供 `Set-CS2Priority` 函数。游戏启动后在同一台机器另一个 PowerShell 跑:
```powershell
. .\cs2_windows_optimize.ps1   # 重新加载函数 (或新开管理员窗口直接粘贴函数)
Set-CS2Priority
```
> 不要设成"实时", 会导致系统/驱动响应不过来反而更卡。High 已足够。

### 7.4 温度与降频
- 用 MSI Afterburner / HWiNFO 监控 GPU/CPU 温度, 游戏中 > 85°C 通常在降频
- 清灰、改善风道、必要时降压超频/降频稳温
- 笔记本: 电源接好、性能模式、垫高底部进风

### 7.5 闪退排查
- 验证游戏完整性: Steam → CS2 右键 → 属性 → 已安装文件 → 验证
- 关闭 MSI Afterburner / RTSS 的 OSD 注入 (偶尔与 CS2 反作弊冲突)
- 关闭 Overwolf、Discord Overlay 等第三方叠加测试
- 闪退看 `game\csgo\` 下 `crash_*.dmp` 或事件查看器

---

## 8. 验证效果

1. 进游戏 → 设置 → 视频 → 右上角开启 **FPS / Ping / 丢包 / 抖动** 显示
2. 死斗或离线跑图 5~10 分钟, 观察:
   - FPS 是否稳定 (目标: ≥ 刷新率; 1% low 帧数尽量不掉过刷新率 70%)
   - Ping 是否平稳, 丢包 0%
   - 有无突发掉帧 (通常对应着色器编译或后台进程)
3. 用 [CapFrameX](https://www.capframex.com/) 或 Steam 内置帧图记录 1% low / 0.1% low
4. 输入延迟可参考 NVIDIA Reflex 统计 (游戏内开启 Reflex 后会显示 System Latency)

---

## 9. 一页速查 (TL;DR)

- [x] 跑 `cs2_windows_optimize.ps1` → 重启
- [x] NVIDIA 驱动最新 + 控制面板: 电源=最高性能, 低延迟=Ultra, VSync=关, 纹理过滤=高性能
- [x] Steam 启动项: `-novid -high -nojoy -fullscreen +fps_max 0 +exec autoexec.cfg`
- [x] `cs2_autoexec.cfg` 放到 `game\csgo\cfg\`
- [x] 游戏内: 全屏 / Reflex=启用+Boost / VSync=关 / 阴影高 / 其余低 / 环境光遮蔽关
- [x] 关 Windows 鼠标加速, 鼠标 1000Hz
- [x] 有线网络, 关后台占用
- [x] 监控温度与 1% low 帧
