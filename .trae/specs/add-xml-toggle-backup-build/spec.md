# XML 修改开关 + 原图备份 + 实际构建 Spec

## 技术报告（设计分析）

### 1. 需求理解

在已实现的图片拼接软件基础上做三项增量增强：

| 需求 | 说明 |
| --- | --- |
| 1. XML 修改开关 | 右下角加复选框"同时修改 XML"，不勾选则输出时只更新图片文件，不修改源 XML |
| 2. 原图备份 | 新增备份文件夹（与原图片文件夹同级），存储被覆盖/删除的原图，作为撤销手段 |
| 3. 实际构建 | 下载依赖项（CMake + Qt5.15.2 + MinGW），编译修复所有报错，生成可运行 exe |

### 2. 关键设计决策

**(A) 复选框 UI 位置**：将右侧展示区改为 `QVBoxLayout`：上方 `m_displayLabel`（Stretch=1）+ 下方一行 `QHBoxLayout`（addStretch + `QCheckBox` 右对齐）。复选框默认勾选（保持原行为）。文案"同时修改 XML"，tooltip 说明勾选/不勾选的差异。此布局"右下"位置清晰、不遮挡图片、与左下"输出"按钮视觉对应，符合人体工学。

**(B) 备份目录规则**：与原图片文件夹同级，名称 = 原文件夹名 + `_backup`。
- 例：`D:\pinjie\imageCut` → 备份到 `D:\pinjie\imageCut_backup`
- 计算：`QFileInfo(folder).absolutePath()` + `/` + `QFileInfo(folder).fileName()` + `_backup`
- 目录不存在则自动创建（`QDir::mkpath`）

**(C) 备份时机与对象**：在覆盖/删除**之前**先备份。
- 备份 `keepPath`（小序号原图，将被覆盖）
- 备份 `removePath`（大序号原图，将被删除）
- 用 `QFile::copy(src, dst)`，目标已存在则先 `QFile::remove(dst)` 再复制（保证最新原图覆盖旧备份）
- 备份失败 → 弹错并 return，不继续覆盖/删除（保护原图优先）

**(D) 输出流程新顺序**（含条件分支）：
```
1. 确认对话框（文案根据 modifyXml 调整，含备份提示）
2. backupDir = FileManager::backupDirFor(folder)
3. FileManager::backupFile(keepPath, backupDir)    // 备份将被覆盖的小序号原图
4. FileManager::backupFile(removePath, backupDir)  // 备份将被删除的大序号原图
5. FileManager::saveImage(keepPath, stitched)      // 覆盖小序号图
6. FileManager::deleteFile(removePath)             // 删除大序号图
7. if (modifyXml) XmlEditor::updateAndRemove(...)  // 条件修改 XML
```

**(E) 构建方案 —— MinGW 路线（轻量可行）**：
- 本机仅有 pip（Python 3.12），无 cmake/Qt/编译器。
- **CMake**：`pip install cmake`（最省事）
- **Qt5.15.2 + MinGW**：`pip install aqtinstall`，然后 `aqt install-qt windows desktop 5.15.2 win64_mingw81_64` + `aqt install-tool windows desktop tools_mingw`。aqtinstall 是 Qt 官方推荐命令行安装工具，无人值守、体积可控（~600MB）。
- **为何选 MinGW 而非 MSVC**：MSVC Build Tools 体积 4-8GB、安装 30min+、可能需重启；MinGW 经 aqtinstall 一条命令装齐 Qt+编译器，无人值守。Qt 5.15.2 官方支持 Win7 SP1，MinGW-W64 8.1 生成代码 Win7 兼容，运行时 DLL 由 windeployqt 随包分发即可。
- **CMakeLists 适配**：`/utf-8` 已用 `if(MSVC)` 隔离，MinGW 不受影响。MinGW Makefiles 生成器无需改 CMakeLists。
- **打包**：`windeployqt --release ImageStitcher.exe`，自动打包 Qt5Core/Gui/Widgets/Xml/Concurrent.dll、platforms/qwindows.dll、imageformats/qjpeg.dll、MinGW 运行时（libgcc_s_seh-1.dll、libstdc++-6.dll、libwinpthread-1.dll）。

**(F) 高内聚低耦合**：
- 备份逻辑归属 `FileManager`（文件操作的本职），新增 3 个静态方法：`backupDirFor` / `backupFile` / `backupFiles`。
- `MainWindow` 仅调用，不含备份实现细节。
- `XmlEditor` / `Stitcher` / `NumberUtil` 不变。
- XML 条件分支仅在 `MainWindow::onOutput` 中以 `if (modifyXml)` 控制，不污染服务层。

### 3. 数据库范式说明

本项目无数据库。XML 结构中每个 `<插图>` 仅描述单张图片属性，无传递/部分依赖，符合 BC 范式精神。本次增量不改变 XML 结构。

---

## Why

用户需要更安全的输出流程：可选择性跳过 XML 修改（仅图片处理场景），并自动备份原图以便误操作后撤销。同时需将代码实际编译为可运行 exe 验证功能。

## What Changes

- **MODIFIED** `FileManager`：新增 3 个静态备份方法（`backupDirFor` / `backupFile` / `backupFiles`）
- **MODIFIED** `MainWindow` UI：右侧展示区下方加"同时修改 XML"复选框（右下角，默认勾选）
- **MODIFIED** `MainWindow::onOutput`：输出流程增加备份步骤（先于覆盖/删除）+ XML 修改条件分支 + 确认对话框文案动态化
- **NEW** 构建环境：pip 安装 cmake + aqtinstall；aqt 安装 Qt5.15.2 mingw81_64 + MinGW 工具链
- **NEW** 编译打包：CMake 配置 + MinGW 编译 + windeployqt 打包生成 exe

## Impact

- Affected code: [FileManager.h/.cpp](file:///d:/pinjie/src/FileManager.h)、[MainWindow.h/.cpp](file:///d:/pinjie/src/MainWindow.h)
- Affected specs: `implement-image-stitcher`（增量扩展，不破坏既有功能）
- 新增产物：`d:\pinjie\src\build\` 构建目录、`ImageStitcher.exe` 及依赖 DLL 包
- 平台：Win7 SP1+ 兼容性保持（Qt5.15 + `_WIN32_WINNT=0x0601`，MinGW 运行时随包分发）

## ADDED Requirements

### Requirement: XML 修改开关

系统 SHALL 在 UI 右下角提供"同时修改 XML"复选框（默认勾选）。输出时若复选框未勾选，系统 SHALL 仅执行图片文件更新（覆盖小序号图、删除大序号图、备份原图），SHALL NOT 修改源 XML。若勾选，SHALL 同步修改源 XML（更新宽高 + 删除大序号段落）。

#### Scenario: 不勾选时仅更新图片
- **GIVEN** 用户预览后取消勾选"同时修改 XML"，点击输出并确认
- **THEN** 小序号图被拼接图覆盖、大序号图被删除、原图被备份；源 XML 内容不变

#### Scenario: 勾选时同步修改 XML（默认行为）
- **GIVEN** 用户保持勾选"同时修改 XML"，点击输出并确认
- **THEN** 图片更新 + 原图备份 + 源 XML 被修改（小序号图宽高更新、大序号图段落删除）

### Requirement: 原图备份

系统 SHALL 在输出时，于覆盖/删除前，将被覆盖的小序号原图与被删除的大序号原图复制到备份文件夹。备份文件夹位于原图片文件夹同级目录，名称为原文件夹名 + `_backup`。备份目录不存在时 SHALL 自动创建。

#### Scenario: 首次备份
- **GIVEN** 图片文件夹 `D:\pinjie\imageCut`，输出 img00024002 + img00024003
- **WHEN** 点击输出并确认
- **THEN** 创建 `D:\pinjie\imageCut_backup`，其中包含 img00024002.jpg（覆盖前原图）与 img00024003.jpg（删除前原图）；随后原文件夹中小序号图被覆盖、大序号图被删除

#### Scenario: 重复备份覆盖旧备份
- **GIVEN** 备份文件夹已存在同名备份文件
- **WHEN** 再次输出
- **THEN** 旧备份文件被新原图覆盖（保证备份为最新原图）

#### Scenario: 备份失败保护原图
- **WHEN** 备份操作失败（如权限不足）
- **THEN** 弹错误提示，终止输出流程，不覆盖/删除原图

### Requirement: 实际编译生成可运行 exe

系统 SHALL 通过下载安装构建工具链（CMake via pip、Qt5.15.2 mingw81_64 via aqtinstall、MinGW 工具链），配置 CMake（MinGW Makefiles 生成器），编译项目修复所有报错，并用 windeployqt 打包，最终生成可在 Windows 7 SP1+ 运行的 `ImageStitcher.exe` 及其依赖 DLL。

#### Scenario: 成功生成 exe
- **WHEN** 完成工具链安装、CMake 配置、编译、windeployqt 打包
- **THEN** `d:\pinjie\src\build\` 下生成 `ImageStitcher.exe`，双击可启动，能完成预览与输出全流程，无缺 DLL 报错

## MODIFIED Requirements

### Requirement: 拼接输出与 XML 修正（扩展备份与开关）

系统 SHALL 在点击输出时：（1）弹确认对话框，文案根据是否勾选"同时修改 XML"动态显示操作项，含备份提示；（2）计算备份目录 = 原图片文件夹同级 + `_backup`；（3）先备份小序号原图与大序号原图到备份目录（失败则终止）；（4）保存拼接图覆盖小序号文件；（5）删除大序号文件；（6）若勾选"同时修改 XML"，执行 XmlEditor.updateAndRemove，否则跳过；（7）不产生新 XML 文件。

#### Scenario: 完整输出流程（勾选 XML）
- **GIVEN** 预览完成，"同时修改 XML"勾选
- **WHEN** 点击输出并确认
- **THEN** 备份两原图 → 覆盖小序号图 → 删除大序号图 → 修改源 XML（宽高更新+段落删除）；输出成功提示

#### Scenario: 仅图片输出流程（未勾选 XML）
- **GIVEN** 预览完成，"同时修改 XML"未勾选
- **WHEN** 点击输出并确认
- **THEN** 备份两原图 → 覆盖小序号图 → 删除大序号图；源 XML 不变；输出成功提示
