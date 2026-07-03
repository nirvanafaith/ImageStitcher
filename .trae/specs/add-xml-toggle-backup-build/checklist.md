# Checklist

## FileManager 备份功能
- [x] `FileManager.h` 新增 `backupDirFor` / `backupFile` / `backupFiles` 静态方法声明
- [x] `backupDirFor("D:/pinjie/imageCut")` 返回 `D:/pinjie/imageCut_backup`
- [x] `backupFile` 目录不存在时自动创建（QDir::mkpath）
- [x] `backupFile` 目标已存在时先删后复制（保证最新原图覆盖旧备份）
- [x] `backupFile` srcPath 不存在返回 true（视为无需备份）
- [x] `backupFiles` 批量调用 backupFile

## MainWindow UI 复选框
- [x] `MainWindow.h` 新增 `QCheckBox *m_modifyXmlCheck` 成员
- [x] 右侧布局改为 QVBoxLayout（上 displayLabel + 下 复选框右对齐）
- [x] 复选框文本"同时修改 XML"，位于右下角
- [x] 默认勾选（setChecked(true)）
- [x] tooltip 说明勾选/不勾选的差异

## 输出流程改造
- [x] onOutput 读取 `m_modifyXmlCheck->isChecked()`
- [x] 确认对话框文案根据 modifyXml 动态显示（含备份提示）
- [x] 备份步骤在 save/delete 之前执行
- [x] 备份 keepPath 与 removePath 到 backupDir
- [x] 备份失败弹错并 return（不覆盖/删除原图）
- [x] XML 修改在 `if (modifyXml)` 条件分支内
- [x] 未勾选时源 XML 内容不变
- [x] 成功提示文案根据 modifyXml 调整

## 备份机制验证
- [x] 代码逻辑：首次输出后 `D:\pinjie\imageCut_backup` 会被 QDir::mkpath 创建
- [x] 代码逻辑：备份文件夹含 img00024002.jpg 与 img00024003.jpg（输出前原图）
- [x] 代码逻辑：重复输出时旧备份被新原图覆盖（QFile::remove + QFile::copy）
- [x] 代码逻辑：备份失败时 return（不覆盖/删除原图）

## 构建工具链安装
- [x] `pip install cmake` 成功，cmake --version 可用
- [x] `pip install aqtinstall` 成功
- [x] aqtinstall 装好 Qt5.15.2 mingw81_64 到 `d:\pinjie\qt5.15.2\`
- [x] aqtinstall 装好 MinGW 工具链（qt.tools.win64_mingw810）
- [x] qmake、g++、windeployqt 可用（绝对路径调用）

## 编译与生成 exe
- [x] CMake 配置成功（MinGW Makefiles 生成器，Qt5.15.2 路径正确）
- [x] 编译 0 错误（[100%] Built target ImageStitcher）
- [x] `d:\pinjie\src\build\` 下生成 `ImageStitcher.exe`
- [x] windeployqt 打包后依赖 DLL 齐全（Qt5 Core/Gui/Widgets/Xml/Svg + platforms/qwindows + imageformats/qjpeg 等 + MinGW 运行时 libgcc_s_seh-1/libstdc++-6/libwinpthread-1）
- [x] 双击 ImageStitcher.exe 可启动（后台运行无缺 DLL 报错）
- [ ] 可完成预览流程（6 种组合不崩溃）—— 待用户实际操作 UI 验证
- [ ] 可完成输出流程（勾选/不勾选 XML 均正确）—— 待用户实际操作 UI 验证
- [x] Win7 SP1+ 兼容性保持（_WIN32_WINNT=0x0601，Qt5.15，MinGW 运行时随包分发）
