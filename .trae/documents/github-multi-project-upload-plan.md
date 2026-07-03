# GitHub 多项目上传执行计划

## 概述

将 4 个本地项目上传到 GitHub（nirvanafaith 账号下）：

| 序号 | 本地路径 | GitHub 仓库 | 可见性 | 操作 |
|------|----------|-------------|--------|------|
| 1 | `D:\hx` | `PDF-OCR`（已存在） | 维持原状 | force push 覆盖 |
| 2 | `D:\puzzle` | `puzzle`（新建） | 私有 | 新建并推送 |
| 3 | `C:\Users\E-VR\Desktop\pathos remake` | `pathos-remake`（新建） | 私有 | 新建并推送 |
| 4 | `D:\pinjie` | `ImageStitcher`（新建） | 私有 | 新建并推送 |

## 当前状态分析（Phase 1 探索结果）

### Git 环境
- git 版本：2.54.0.windows.1
- 安装路径：`C:\Program Files\Git\bin\`
- 已配置：`safe.directory=*`
- **缺失**：user.name、user.email、credential.helper、.git-credentials 文件
- 需在每条命令前加：`$env:PATH = "C:\Program Files\Git\bin;C:\Program Files\Git\cmd;" + $env:PATH`

### 凭据
- GitHub 用户名：`nirvanafaith`
- PAT：`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`
- 用 `credential.helper store` 明文存储到 `~/.git-credentials`

### 各项目结构概览

#### 1. D:\hx（Python PDF OCR）
- `software1/` - 主程序目录（main.py、ocr_engine/、models/、json/、output/）
- `.trae/documents/` - 60+ 个开发文档（.md）
- `.trae/specs/` - 30+ 个 spec 规范目录
- `.trae/scripts/` - 辅助脚本
- 根目录：`2_红.pdf`、`2_透明.pdf`、`compile_err.txt`、`compile_out.txt`
- **需排除**：`__pycache__/`、`*.pyc`、`software1/output/`（OCR 输出的大量 .jpg 和 .pdf 中间产物）、`software1/json/*/` 中的 `chars.json`/`lines.json`/`newchar.json` 是测试数据可保留

#### 2. D:\puzzle（Godot 连连看，~10.3GB）
- `scenes/` - 14 个 .tscn 场景文件（源码）
- `scripts/` - 40+ 个 .gd GDScript 脚本（源码）
- `src/` - 12 个 .cpp/.h C++ GDExtension 源码
- `assets/audio/` - 12 个 .wav 音频资源（源码）
- `assets/icons/`、`assets/textures/` - 图片资源（源码）
- `CMakeLists.txt`、`CMakePresets.json`、`project.godot`、`lianliankan.gdextension`（源码）
- **需排除**：
  - `.godot/` - 编辑器缓存
  - `build/` - CMake 构建产物（含 .obj、.pdb、.apk）
  - `build-android-*/` - 4 个 Android 构建目录
  - `godot-cpp/` - 第三方库（约几百 MB，可重新克隆）
  - `tools/android-ndk-r25c/` - Android NDK（约 1GB+）
  - `tools/tpl_tmp/` - Godot 导出模板
  - `tools/godot-4.5-templates.tpz` - 模板包
  - `android_templates/` - Android 模板
  - `bin/` - 编译输出（.dll、.pdb、.so）
  - `Godot_v4.5-stable_win64*.exe` - Godot 编辑器本体
  - `keystore/` - 签名密钥（**敏感文件，绝不上传**）
  - `*.apk`、`*.idsig` - APK 包
  - `*.txt` 日志文件（export_log*.txt、godot_*.txt）
  - `.trae/` - 开发文档（可选保留，但占用空间小，保留以便后续查阅）

#### 3. C:\Users\E-VR\Desktop\pathos remake（Godot 地牢游戏）
- `art/` - 美术资源（containers/effects/enemies/icons/items/player/tavern/tiles）（源码）
- `scenes/` - 9 个 .tscn 场景文件（源码）
- `scripts/` - 60+ 个 .gd 脚本（源码，列表被截断）
- `godot-cpp/` - 第三方库（**排除**）
- `SConstruct`、`project.godot`、`pathos_remake.gdextension`（源码）
- `game/pathos_remake.exe` - 编译产物（**排除**）
- **需排除**：
  - `.godot/` - 编辑器缓存
  - `godot-cpp/` - 第三方库
  - `Godot_v4.5-stable_win64*.exe` - Godot 编辑器
  - `game/` - 编译输出目录
  - `.sconsign.dblite` - scons 构建数据库
  - `extension_api.json` - 大文件（Godot API 描述，可重新生成）
  - `*.log`、`godot_*.txt` - 日志文件
  - `*.ps1` - 调试 PowerShell 脚本（alt_tab.ps1、attack_test.ps1、focus_pathos*.ps1、force_fg.ps1、move_right.ps1、resize_window.ps1、run_godot.ps1）
  - `pathos_capture*.png`、`pathos_view.png` - 调试截图
  - `after_attack.png` - 调试截图
  - `.trae/` - 保留（开发文档）

#### 4. D:\pinjie（Qt 5.15.2 图片拼接工具）
- `src/` - C++ 源码（MainWindow、Stitcher、FileManager、XmlEditor、NumberUtil、PathResolver、XmlChecker、CheckResultDialog）+ `src/build/` 构建产物
- `output/` - 已打包的 Win7 兼容可执行文件（ImageStitcher.exe + Qt5 DLLs + 平台插件）
- `.trae/` - 开发文档和 specs
- `qt5.15.2/` - Qt 工具链和 MinGW（约几 GB，**排除**）
- `aqtinstall.log` - 安装日志
- `图片拼接.7z` - 压缩包
- **用户要求**：上传"全部"（src 源码 + output exe/dll + .trae 文档），仅排除 qt5.15.2 第三方工具链
- **需排除**：
  - `qt5.15.2/` - Qt 工具链和 MinGW 编译器（可重新安装）
  - `src/build/` - CMake 构建产物（.obj、.pdb、CMakeCache.txt 等）
  - `aqtinstall.log` - 安装日志
  - `图片拼接.7z` - 7z 压缩包（大文件）

## 执行步骤

### Step 1：配置 Git 全局环境

```powershell
$env:PATH = "C:\Program Files\Git\bin;C:\Program Files\Git\cmd;" + $env:PATH
git config --global user.name "nirvanafaith"
git config --global user.email "nirvanafaith@users.noreply.github.com"
git config --global credential.helper store
# 写入 PAT 到 ~/.git-credentials
"https://nirvanafaith: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@github.com" | Out-File -FilePath "$env:USERPROFILE\.git-credentials" -Encoding ascii -NoNewline
```

### Step 2：用 mcp_GitHub 创建 3 个私有仓库

调用 `create_repository` 工具创建：
1. `puzzle`（私有，描述："Godot 连连看游戏 - C++ GDExtension + GDScript"）
2. `pathos-remake`（私有，描述："Godot 地牢探险游戏 - C++ GDExtension + GDScript"）
3. `ImageStitcher`（私有，描述："Qt5 图片拼接工具 - Win7 兼容"）

**不创建 PDF-OCR**（已存在，URL: `https://github.com/nirvanafaith/PDF-OCR`）

### Step 3：D:\hx → PDF-OCR（force push 覆盖）

#### 3.1 创建 .gitignore
```gitignore
# Python
__pycache__/
*.py[cod]
*$py.class
*.so
.Python
*.egg-info/
.pytest_cache/

# OCR 输出产物
software1/output/
software1/json/

# 构建日志
compile_err.txt
compile_out.txt

# 临时文件
*.tmp
*.bak
```

#### 3.2 Git 操作
```powershell
cd D:\hx
git init -b main
git remote remove origin 2>$null
git remote add origin https://github.com/nirvanafaith/PDF-OCR.git
# 写入 .gitignore（用 Write 工具）
git add -A
git commit -m "覆盖更新：PDF OCR 工具完整源码"
git push -f origin main
```

### Step 4：D:\puzzle → puzzle（新私有仓库）

#### 4.1 创建 .gitignore
```gitignore
# Godot 编辑器缓存
.godot/

# 构建产物
build/
build-android-*/
bin/

# 第三方库（可重新克隆）
godot-cpp/

# Android NDK 和工具
tools/
android_templates/

# Godot 编辑器本体
Godot_v4.5-stable_win64*.exe

# 签名密钥（敏感）
keystore/

# APK 包
*.apk
*.idsig

# 日志文件
*.log
*.txt
export_log*.txt
godot_*.txt

# 临时文件
*.tmp
*.bak
```

#### 4.2 Git 操作
```powershell
cd D:\puzzle
git init -b main
git remote add origin https://github.com/nirvanafaith/puzzle.git
# 写入 .gitignore（用 Write 工具）
git add -A
git commit -m "初始化：Godot 连连看游戏源码（C++ GDExtension + GDScript）"
git push -u origin main
```

### Step 5：pathos remake → pathos-remake（新私有仓库）

#### 5.1 创建 .gitignore
```gitignore
# Godot 编辑器缓存
.godot/

# 第三方库（可重新克隆）
godot-cpp/

# 构建产物
game/
.sconsign.dblite

# Godot 编辑器本体
Godot_v4.5-stable_win64*.exe

# API 描述大文件（可重新生成）
extension_api.json

# 日志文件
*.log
godot_*.txt
godot_*.log

# 调试 PowerShell 脚本
*.ps1

# 调试截图
pathos_capture*.png
pathos_view.png
after_attack.png

# 临时文件
*.tmp
*.bak
```

#### 5.2 Git 操作
```powershell
cd "C:\Users\E-VR\Desktop\pathos remake"
git init -b main
git remote add origin https://github.com/nirvanafaith/pathos-remake.git
# 写入 .gitignore（用 Write 工具）
git add -A
git commit -m "初始化：Godot 地牢探险游戏源码（C++ GDExtension + GDScript）"
git push -u origin main
```

### Step 6：D:\pinjie → ImageStitcher（新私有仓库）

#### 6.1 创建 .gitignore
```gitignore
# Qt 工具链和 MinGW 编译器（第三方，可重新安装）
qt5.15.2/

# CMake 构建产物
src/build/

# 安装日志
aqtinstall.log

# 压缩包
图片拼接.7z

# 临时文件
*.tmp
*.bak
```

#### 6.2 Git 操作
```powershell
cd D:\pinjie
git init -b main
git remote add origin https://github.com/nirvanafaith/ImageStitcher.git
# 写入 .gitignore（用 Write 工具）
git add -A
git commit -m "初始化：Qt5 图片拼接工具（Win7 兼容，含可执行文件）"
git push -u origin main
```

## 假设与决策

1. **PDF-OCR 仓库覆盖策略**：使用 `git push -f` force push 覆盖远程所有内容。这会丢失远程仓库的提交历史，符合用户"覆盖"的要求。
2. **PAT 存储**：用 `credential.helper store` 明文存储到 `~/.git-credentials`。虽不安全，但用户已确认此方案。
3. **D:\hx 测试数据**：`software1/json/` 中的 chars.json/lines.json 是 OCR 测试数据，体积可能较大，排除以减小仓库体积。`.trae/` 开发文档保留（体积小，有参考价值）。
4. **D:\puzzle 第三方库**：`godot-cpp/` 是 Godot 官方 C++ 绑定库，可从 GitHub 重新克隆，排除以避免仓库膨胀。
5. **D:\puzzle keystore**：`keystore/lianliankan.keystore` 是 Android 签名密钥，**绝不上传**（敏感信息）。
6. **pathos remake 调试文件**：`*.ps1`、`*.log`、`godot_*.txt`、调试截图都是开发过程中的临时调试文件，排除。
7. **D:\pinjie 上传范围**：用户明确要求"全部"，包括 src 源码 + output 已打包 exe/dll + .trae 文档。仅排除 qt5.15.2 工具链、src/build/ 构建产物、aqtinstall.log、图片拼接.7z。
8. **分支名**：所有仓库使用 `main` 分支（GitHub 默认）。
9. **提交信息语言**：中文（与用户沟通语言一致）。
10. **执行顺序**：先配置 Git 环境 → 创建 3 个私有仓库 → 按顺序上传 4 个项目。每个项目独立执行，互不影响。

## 验证步骤

每个项目上传完成后验证：

1. **本地验证**：
   ```powershell
   git log --oneline -1  # 确认提交存在
   git remote -v         # 确认远程地址正确
   ```

2. **远程验证**（用 mcp_GitHub 工具）：
   - `get_file_contents` 检查关键文件是否上传成功
   - 对 puzzle/pathos-remake/ImageStitcher 验证仓库 `private` 属性为 true

3. **PDF-OCR 验证**：
   - 用 `list_commits` 确认远程只有 1 个新提交（force push 后历史被覆盖）
   - 用 `get_file_contents` 检查 software1/main.py 是否存在

4. **私有仓库可见性验证**：
   - 确认 puzzle、pathos-remake、ImageStitcher 三个仓库的 `private: true`
   - 他人无法访问（用未登录状态访问仓库 URL 应返回 404）

## 风险与注意事项

1. **force push 风险**：PDF-OCR 的 force push 会永久删除远程历史。已确认用户要求"覆盖"。
2. **PAT 泄露风险**：PAT 会明文存储在 `~/.git-credentials`。任务完成后建议用户考虑撤销该 PAT 并重新生成。
3. **大文件风险**：尽管已排除主要大文件，仍可能有少量 .png/.wav 资源超过 GitHub 的 100MB 单文件限制。若 push 失败，需用 `git rm --cached` 排除大文件后重新提交。
4. **路径含空格**：`C:\Users\E-VR\Desktop\pathos remake` 路径含空格，PowerShell 命令需用引号包裹路径。
5. **执行时间**：4 个项目串行执行，puzzle 项目即使排除大文件后仍可能有较多小文件，push 时间可能较长。

## 执行顺序总览

1. 配置 Git 全局环境（user.name/email/credential.helper/PAT）
2. 用 mcp_GitHub 创建 3 个私有仓库（puzzle、pathos-remake、ImageStitcher）
3. 上传 D:\hx → PDF-OCR（force push 覆盖）
4. 上传 D:\puzzle → puzzle
5. 上传 pathos remake → pathos-remake
6. 上传 D:\pinjie → ImageStitcher
7. 验证所有仓库上传成功且私有仓库不可被他人访问
