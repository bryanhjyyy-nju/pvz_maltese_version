# 小白大战小金毛

## 游戏介绍

一款受《植物大战僵尸》玩法启发的 C++ / Qt 塔防小游戏。玩家在草坪上部署不同能力的小白，收集爱心补充资源，抵挡逐波来袭的小金毛。

- **关卡模式**：共 10 关，逐关解锁小白与新的敌人。前三关逐步开放一排、三排、五排草坪，前两关提供操作教程。
- **无尽模式**：通关第十关后解锁，每四个小波接一个大波，随波次推进增加进攻压力，并记录最高波数。失败后返回主菜单，可重新开始无尽模式。
- **角色图鉴**：包含 8 种小白和 3 种金毛，可查看生命、攻击、费用及技能说明。
- **游戏功能**：支持暂停、重新开始、1× / 2× 速度切换、全屏，以及分别调整背景音乐和音效音量。
- **存档续玩**：保存关卡进度；战斗中正常关闭窗口会保存场上局势，再次点击“继续游戏”后从暂停状态恢复。开场或教程阶段退出时，继续会重新进入该关开场并暂停。

### 基本操作

| 操作 | 功能 |
| --- | --- |
| 鼠标左键 | 选择小白卡片、在可用草格部署小白、收集爱心 |
| 鼠标右键 | 取消选中的小白、铲子或手套移动 |
| 铲子按钮 / `R` | 拿起或放下铲子；拿起后点击小白将其移除，第二关起可用 |
| 手套按钮 / `S` | 无尽模式移动已有小白：先选小白，再点空草格落位；再次按 `S` 或右键取消 |
| 空格 / 暂停按钮 | 暂停或继续游戏 |
| 速度按钮 / `F` | 切换 1× / 2× 速度 |
| `A` / `D` | 分别开关小白 / 金毛的血量显示 |
| `F11` / 全屏按钮 | 切换全屏 |
| `Esc` | 退出全屏 |

暂停菜单可以重新开始当前游戏、返回主菜单、查看图鉴和调整声音。暂停会保留已选小白、铲子或手套移动状态，继续后可以接着操作。

无尽模式的手套位于铲子旁边。选中小白后，半透明的静止幻影随手套和指针移动；原小白在原格继续战斗，点击可用空草格才搬到新位置。搬动保留生命值和活动进度，也不会消耗爱心或触发卡片冷却。暂停或存档续玩会保留待搬动状态；冲锋小白不能用手套移动或用铲子移除。

存档通常位于 Windows 的 `%APPDATA%/DogGarden/PvZ_demo/progress.json`。主菜单的“存档管理”可清空进度或开放全部关卡。

## 代码介绍

项目使用 **C++17、Qt Widgets 和 Graphics View**，通过 qmake 构建。源文件与头文件分别放在 `src/` 和 `include/` 中。

```text
pvz_maltese_version/
├── src/                    游戏源文件（.cpp）
│   └── tests/              自动化测试与性能基准源码
├── include/                游戏头文件（.h）
├── Image/                  图片与动画资源
├── Media/                  音乐与音效资源
├── docs/                   关卡调参、平衡与性能说明
├── tools/                  构建脚本与音频生成工具
├── res.qrc                 Qt 资源清单
├── PvZ_demo.pro            游戏工程
├── pvz_tests.pro           测试工程
└── pvz_benchmarks.pro      性能基准工程
```

### 主要模块

下表中的模块各有对应的 `src/模块名.cpp` 和 `include/模块名.h`。

| 模块 | 职责 |
| --- | --- |
| `gamewindow`、`gamepage` | 主窗口、页面切换、画面缩放和全屏管理 |
| `mainscene`、`chooselevelscene` | 主菜单、关卡选择和进度展示 |
| `playscene`、`mygamescene` | 战斗界面、草坪场景、单位部署、刷怪和胜负判断 |
| `gamecatalog`、`waveplanner` | 角色与关卡数值、波次阵容、敌人生成和间隔安排 |
| `whitedogs`、各小白类、`yellowdogs` | 小白与金毛的行为、攻击和碰撞 |
| `card`、`heart`、`bullet`、`enemyprojectile` | 卡片、爱心和双方弹丸 |
| `gamespeed`、`gamepause`、`spriteanimation` | 倍速、暂停恢复和角色动画 |
| `lawn`、`map`、`levelopening`、`leveltutorial` | 草坪布局、开场动画和操作教程 |
| `battlebanner`、`battleresult`、`combateffect` | 波次提示、结算和战斗特效 |
| `progressstore`、`battlesnapshot` | 关卡进度与完整战场的保存、校验和恢复 |
| `almanacdialog`、`audiomanager` | 图鉴、音乐音效与音量设置 |
| `gameui`、`gameartwork`、`pausedialog` | 共用界面样式、绘图和暂停菜单 |

游戏入口为 `src/main.cpp`。修改角色或关卡数值可从 `src/gamecatalog.cpp` 开始，波次生成逻辑位于 `src/waveplanner.cpp`；详细说明见 [调参指南](docs/TUNING.md)。

图片和声音通过 `res.qrc` 打包进程序。`Media/*.wav` 由 `tools/generate_audio.py` 合成；木框和星星素材来自 Kenney UI Pack — Adventure，CC0 许可与来源说明保存在 `Image/ui/`。

## 编译与使用

### 环境要求

已验证的构建环境为 **Windows、Qt 5.15.2、MinGW 8.1（64 位）**。

- C++17 编译器与 qmake。
- Qt Core、Gui、Widgets、Multimedia 模块。
- 构建测试或性能基准时还需要 Qt Test。
- 使用 `tools/build.ps1` 时需要 PowerShell。

### 获取代码

```powershell
git clone https://github.com/bryanhjyyy-nju/pvz_maltese_version.git
Set-Location pvz_maltese_version
```

以下命令均在项目根目录运行。

### 用构建脚本编译

```powershell
.\tools\build.ps1 -Deploy
.\build-release\dist\PvZ_demo.exe
```

脚本默认使用 `C:\Qt\5.15.2\mingw81_64` 和 `C:\Qt\Tools\mingw810_64`。安装位置不同时，可指定路径：

```powershell
.\tools\build.ps1 -Deploy `
    -QtRoot 'D:\Qt\5.15.2\mingw81_64' `
    -CompilerRoot 'D:\Qt\Tools\mingw810_64'
```

`-Deploy` 会将程序、Qt 运行库和插件复制到 `build-release/dist/`。运行或分享游戏时，保留这个目录中的全部文件。

仅编译时运行 `.\tools\build.ps1`，可执行文件生成在 `build-release/release/PvZ_demo.exe`，运行时需自行提供 Qt 运行库与插件。

### 用 Qt Creator 编译

1. 打开根目录的 `PvZ_demo.pro`。
2. 选择包含 Multimedia 模块的 Qt 5.15.2 / MinGW 64 位 Kit。
3. 选择 Release 配置，构建并运行。

### 运行测试

```powershell
.\tools\build.ps1 -Test
```

测试报告生成在 `build-qt5-tests/test-results.txt`，界面截图位于 `build-qt5-tests/screenshots/`。脚本使用离屏模式运行，Windows 窗口像素检查会跳过。

### 性能基准

可在 Qt Creator 中打开 `pvz_benchmarks.pro`，或在独立目录中构建运行：

```powershell
New-Item -ItemType Directory -Force build-benchmarks
Set-Location build-benchmarks
$env:PATH = 'C:\Qt\5.15.2\mingw81_64\bin;C:\Qt\Tools\mingw810_64\bin;' + $env:PATH
qmake ..\pvz_benchmarks.pro 'CONFIG+=release' 'CONFIG-=debug'
mingw32-make -j4
.\release\pvz_benchmarks.exe -platform windows -o benchmark-results.txt,txt
```

Qt 安装在其他位置时，调整命令中的工具链路径。测量说明见 [性能文档](docs/PERFORMANCE.md)。
