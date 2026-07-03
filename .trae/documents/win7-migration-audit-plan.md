# Win7 迁移兼容性审查与修复计划

## Summary

对现有 C++/Qt5 图片拼接软件进行全面 Win7 兼容性审查。审查结论：**代码层面基本已兼容 Win7 SP1，但 MinGW 构建缺少 `-mwindows` 链接选项，导致双击 exe 会弹出控制台窗口**。本计划将修复该问题，并给出完整的 Win7 部署清单。

---

## Current State Analysis（基于 Phase 1 探索）

### 已读关键文件
- [CMakeLists.txt](file:///d:/pinjie/src/CMakeLists.txt)：构建配置
- [main.cpp](file:///d:/pinjie/src/main.cpp)：程序入口
- [Stitcher.cpp](file:///d:/pinjie/src/Stitcher.cpp)：图像拼接算法
- [FileManager.cpp](file:///d:/pinjie/src/FileManager.cpp)：文件/备份/并行加载
- [MainWindow.cpp](file:///d:/pinjie/src/MainWindow.cpp)：UI 主窗口
- [NumberUtil.cpp](file:///d:/pinjie/src/NumberUtil.cpp)：序号解析/补全
- [XmlEditor.cpp](file:///d:/pinjie/src/XmlEditor.cpp)：XML 修改
- [PathResolver.cpp](file:///d:/pinjie/src/PathResolver.cpp)：智能导入路径解析
- [XmlChecker.cpp](file:///d:/pinjie/src/XmlChecker.cpp)：图片名校对
- [CheckResultDialog.cpp](file:///d:/pinjie/src/CheckResultDialog.cpp)：校对结果弹窗

### 已确认兼容项

| 检查项 | 状态 | 说明 |
|---|---|---|
| Qt 版本 | ✅ | Qt 5.15.2 是最后一个官方支持 Win7 SP1 的 LTS 版本 |
| 编译器 | ✅ | MinGW 8.1.0 64-bit 与 Qt5.15.2 官方二进制配套 |
| C++ 标准 | ✅ | C++17，MinGW 8.1 完全支持 |
| Win7 API 目标版本 | ✅ | 已设置 `_WIN32_WINNT=0x0601 WINVER=0x0601` |
| NOMINMAX | ✅ | 已设置，避免 Windows 宏与 `std::min/max` 冲突 |
| Qt API 版本 | ✅ | 全部使用 Qt5 API（QDomDocument、QImage、QRegularExpression、QtConcurrent、QDirIterator 等） |
| 直接 Windows API 调用 | ✅ | 无，全部通过 Qt 跨平台 API |
| 中文路径/Unicode | ✅ | 使用 `QString` + `QFile`，Qt 内部处理 Unicode |
| 源码编码 | ✅ | MSVC 已加 `/utf-8`，MinGW 默认 UTF-8 |
| XML 编码 | ✅ | 使用 `QTextStream::setCodec("UTF-8")`，Qt5 支持 |
| 线程并行 | ✅ | `QtConcurrent::run` 函数指针重载在 Qt5.15 中可用 |

### 发现的问题

#### 问题 1：MinGW 构建未隐藏控制台窗口（需修复）
**位置**：[CMakeLists.txt](file:///d:/pinjie/src/CMakeLists.txt#L55-L57)

```cmake
if(MSVC)
    set_target_properties(ImageStitcher PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
```

**影响**：`WIN32_EXECUTABLE` 属性仅在 MSVC 下设置。MinGW 构建的 exe 在 Win7 上双击运行时会同时弹出一个黑色命令行控制台窗口，严重影响使用体验。

**修复**：为 MinGW 添加 `-mwindows` 链接选项：
```cmake
if(MINGW)
    target_link_options(ImageStitcher PRIVATE -mwindows)
endif()
```

#### 问题 2：Qt5.15.2 在 Win7 上的系统补丁要求（部署注意，非代码问题）
Qt5.15.2 的 Windows 平台插件基于 DXGI 1.1，要求 Win7 SP1 + Platform Update（KB2670838）。未安装该补丁的旧 Win7 可能出现：
- "无法定位程序输入点"错误
- "无法加载平台插件 qwindows"错误

此问题通过部署说明解决，非代码修改。

#### 问题 3：MinGW 运行时 DLL 需随 exe 分发（部署注意，非代码问题）
MinGW 构建依赖以下运行时 DLL，需放入 exe 同级目录：
- `libgcc_s_seh-1.dll`
- `libstdc++-6.dll`
- `libwinpthread-1.dll`

这些已通过 windeployqt + 手动复制覆盖。

---

## Proposed Changes

### 修改文件 1: `d:\pinjie\src\CMakeLists.txt`
**What**: 在 MSVC 的 `WIN32_EXECUTABLE` 分支之后，增加 MinGW 的 `-mwindows` 链接选项。
**Why**: 消除 Win7 上双击 exe 时弹出的控制台窗口。
**How**:
```cmake
# MSVC 输出 WIN32 可执行（带 GUI，无控制台窗口）
if(MSVC)
    set_target_properties(ImageStitcher PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# MinGW 同样隐藏控制台窗口
if(MINGW)
    target_link_options(ImageStitcher PRIVATE -mwindows)
endif()
```

---

## Assumptions & Decisions

1. **只修复代码层面的 Win7 兼容问题**：当前唯一代码问题是 MinGW 控制台窗口。
2. **Qt5.15.2 工具链不变**：用户已安装 Qt5.15.2 MinGW 版，继续使用。
3. **部署补丁要求以文档形式说明**：不修改代码或安装程序，仅在最终回复中列出。
4. **`-mwindows` 是 MinGW 标准做法**：不影响调试能力，只在链接阶段去除对 `main` 的控制台依赖。
5. **数据库 BC 范式**：本项目无数据库，XML 结构无传递依赖。

---

## Verification Steps

1. **修改 CMakeLists.txt** 后重新配置并编译：
   ```powershell
   $env:PATH = "d:\pinjie\qt5.15.2\Tools\mingw810_64\bin;d:\pinjie\qt5.15.2\5.15.2\mingw81_64\bin;" + $env:PATH
   cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=d:/pinjie/qt5.15.2/5.15.2/mingw81_64 -S d:\pinjie\src -B d:\pinjie\src\build
   cmake --build d:\pinjie\src\build -j
   ```
2. **编译验证**：0 错误，生成 `ImageStitcher.exe`。
3. **打包验证**：运行 windeployqt，确保 DLL 齐全。
4. **启动验证**：双击 `ImageStitcher.exe`，确认：
   - 没有黑色控制台窗口弹出
   - GUI 主窗口正常显示
   - 无缺 DLL 报错
5. **功能回归验证**：
   - 智能导入根目录
   - 补全按钮
   - 预览/输出（含去白边、拉伸对齐、XML 开关、备份）
   - 校对图片

---

## Win7 部署清单（最终交付时提供给用户）

目标系统要求：
- Windows 7 SP1（32/64 位取决于构建架构，当前为 64 位）
- Windows 7 Platform Update（KB2670838）

exe 同级目录需包含：
- `ImageStitcher.exe`
- Qt5 DLL：`Qt5Core.dll`、`Qt5Gui.dll`、`Qt5Widgets.dll`、`Qt5Xml.dll`、`Qt5Concurrent.dll`
- 平台插件：`platforms/qwindows.dll`
- 图片插件：`imageformats/qjpeg.dll`
- MinGW 运行时：`libgcc_s_seh-1.dll`、`libstdc++-6.dll`、`libwinpthread-1.dll`
