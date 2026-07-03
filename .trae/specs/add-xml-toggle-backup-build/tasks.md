# Tasks

- [x] Task 1: FileManager 增加备份功能
  - [x] SubTask 1.1: 在 `FileManager.h` 新增 3 个静态方法声明：`backupDirFor(const QString& folder) -> QString`（计算备份目录路径）、`backupFile(const QString& srcPath, const QString& backupDir) -> bool`（复制单文件到备份目录，目录不存在则创建，目标已存在先删后复制）、`backupFiles(const QString& folder, const QStringList& fileNames) -> bool`（便捷批量备份）
  - [x] SubTask 1.2: 在 `FileManager.cpp` 实现上述方法。backupDirFor 用 QFileInfo 取父路径+文件夹名拼接 `_backup`；backupFile 用 QDir::mkpath 创建目录、QFile::copy 复制（目标存在先 QFile::remove）；srcPath 不存在返回 true
- [x] Task 2: MainWindow UI 增加"同时修改 XML"复选框
  - [x] SubTask 2.1: 在 `MainWindow.h` 新增成员 `QCheckBox *m_modifyXmlCheck;`，前置声明 QCheckBox
  - [x] SubTask 2.2: 在 `MainWindow.cpp` 构造函数将右侧布局改为 QVBoxLayout（上 m_displayLabel Stretch=1，下 QHBoxLayout: addStretch + m_modifyXmlCheck 右对齐），复选框文本"同时修改 XML"，默认 setChecked(true)，加 tooltip 说明
- [x] Task 3: MainWindow onOutput 流程改造
  - [x] SubTask 3.1: 修改 onOutput：读取 `bool modifyXml = m_modifyXmlCheck->isChecked();`
  - [x] SubTask 3.2: 确认对话框文案动态化：根据 modifyXml 决定是否列出"修改源 XML"，并加"原图将备份至 xxx_backup"提示
  - [x] SubTask 3.3: 在 save/delete 之前插入备份步骤：计算 backupDir=FileManager::backupDirFor(folder)；backupFile(keepPath, backupDir) 与 backupFile(removePath, backupDir)；任一失败弹错并 return（不继续覆盖/删除）
  - [x] SubTask 3.4: XML 修改加条件分支：`if (modifyXml) { if (!XmlEditor::updateAndRemove(...)) {弹错; return;} }`
  - [x] SubTask 3.5: 成功提示文案根据 modifyXml 调整
- [x] Task 4: 安装构建工具链
  - [x] SubTask 4.1: `pip install cmake` 安装 CMake
  - [x] SubTask 4.2: `pip install aqtinstall` 安装 aqtinstall
  - [x] SubTask 4.3: `aqt install-qt windows desktop 5.15.2 win64_mingw81_64` + `aqt install-tool windows desktop tools_mingw qt.tools.win64_mingw810`（装 Qt5.15.2 MinGW64 版 + MinGW 工具链）到 `d:\pinjie\qt5.15.2\`
  - [x] SubTask 4.4: 验证 cmake、qmake、g++、windeployqt 可用
- [x] Task 5: CMake 配置 + 编译 + 修复报错
  - [x] SubTask 5.1: `cmake -S d:\pinjie\src -B d:\pinjie\src\build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=d:/pinjie/qt5.15.2/5.15.2/mingw81_64 -DCMAKE_BUILD_TYPE=Release`
  - [x] SubTask 5.2: `cmake --build d:\pinjie\src\build -j` 编译成功
  - [x] SubTask 5.3: 确认 0 错误生成 ImageStitcher.exe
- [x] Task 6: windeployqt 打包 + 验证 exe 运行
  - [x] SubTask 6.1: 进入 build 目录执行 `windeployqt ImageStitcher.exe`（不加 --release 以避开 plugin 误判为 debug 的问题）
  - [x] SubTask 6.2: 验证 platforms/qwindows.dll、imageformats/qjpeg.dll、Qt5Core/Gui/Widgets/Xml.dll、MinGW 运行时 DLL 齐全
  - [x] SubTask 6.3: 启动 ImageStitcher.exe 验证启动正常（GUI 启动后稳定等待用户操作，无缺 DLL 报错）

# Task Dependencies
- [Task 3] depends on [Task 1, Task 2]
- [Task 5] depends on [Task 1, Task 2, Task 3, Task 4]
- [Task 6] depends on [Task 5]
- [Task 1/2/4] 可并行启动
