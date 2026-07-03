# 智能导入路径 + 图片校对功能 计划

## Summary

在现有图片拼接工具基础上新增两项功能：
1. **智能导入**：新增「智能导入」按钮，用户选择一个根目录后，软件递归向下查找 `imageCut` 文件夹和第一个 `.xml` 文件，一次性填充「图片文件夹」和「XML 文件」两个输入框。保留原有两个「浏览...」按钮作为手动后备。
2. **图片校对**：左侧面板下方新增「校对图片」按钮，比较 `imageCut` 文件夹中的 jpg 文件名集合与 XML 中引用的图片名集合，用自定义对话框分两栏展示「只在 XML 中」和「只在图片库中」的差异列表。

测试范例：`C:\Users\E-VR\Desktop\06\C语言程序设计案例教程97871133186110000（1-1）`（结构为 `根目录/LSF/XML/imageCut/` + `根目录/LSF/XML/xxx.xml`，xml 用英文标签 `<imglink>`，含 806 个图片引用）。

---

## Current State Analysis（基于 Phase 1 探索）

### 现有代码结构
- [MainWindow.h](file:///d:/pinjie/src/MainWindow.h)：含 `m_folderEdit`/`m_xmlEdit` 两个 QLineEdit，`onBrowseFolder()`/`onBrowseXml()` 两个独立槽函数（各弹一个 QFileDialog 选目录/文件，只填一个框）
- [MainWindow.cpp](file:///d:/pinjie/src/MainWindow.cpp#L56-L80)：左侧面板布局为 folderEdit+浏览按钮、xmlEdit+浏览按钮、img1/img2、方向、对齐、预览、输出、stretch
- [XmlEditor.cpp](file:///d:/pinjie/src/XmlEditor.cpp)：**硬编码中文标签** `<插图>`/`<图片链接>`/`<图片宽度>`/`<图片高度>`/`<段落>`，对新范例 xml（英文标签 `<imglink>`）不兼容。本次不改动 XmlEditor，校对功能用独立正则方案绕过标签依赖
- [FileManager.h](file:///d:/pinjie/src/FileManager.h)：含 loadImage/saveImage/deleteFile/joinPath/backup 等静态方法，无扫描文件夹 jpg 的方法
- [NumberUtil](file:///d:/pinjie/src/NumberUtil.cpp)：用正则 `img(\d+)\.jpg` 提取序号，证明图片名格式固定为 `img`+数字+`.jpg`

### 范例验证（Phase 1 Grep 结果）
- 范例 xml 中 `<imglink>` 标签出现 806 次
- 正则 `img\d+\.jpg` 全文匹配 806 次，**0 误匹配**（与 imglink 数量完全一致）
- 中文标签 `<插图>`/`<图片链接>` 在范例 xml 中 0 次
- 结论：正则全文扫描是可靠的图片名提取方案，与标签名解耦，兼容中英文标签

### 关键约束
- Win7 SP1+ 兼容（Qt5.15.2 + MinGW，QDirIterator/QRegularExpression/QDialog 均为 Qt5 早期 API）
- 高内聚低耦合：新功能封装为独立工具类，不污染现有 Stitcher/XmlEditor/NumberUtil
- 极简 UI：新按钮融入现有左侧面板布局，不破坏视觉一致性
- 性能：范例规模（280 jpg + 1 xml + 806 引用）递归查找 + 正则扫描 + 集合差集总 < 200ms，主线程可接受

---

## Proposed Changes

### 新增文件 1: `d:\pinjie\src\PathResolver.h` / `PathResolver.cpp`
**What**: 静态工具类，提供 `resolveImageCutAndXml(const QString& root, QString& imageCutDir, QString& xmlPath) -> bool`
**Why**: 将"递归查找 imageCut 文件夹和第一个 xml"逻辑高内聚封装，与 UI 解耦
**How**:
- 用 `QDirIterator(root, QDir::Dirs | QDir::NoSymLinks, QDirIterator::Subdirectories)` 遍历，找到第一个名为 `imageCut` 的目录即记录并停止该迭代
- 用 `QDirIterator(root, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories)` 遍历，找到第一个 xml 文件即记录并停止
- 两者都找到返回 true，任一未找到返回 false（调用方据此弹提示）
- 性能：QDirIterator 增量遍历，找到即停，不遍历整棵树

### 新增文件 2: `d:\pinjie\src\XmlChecker.h` / `XmlChecker.cpp`
**What**: 静态工具类，提供两个方法：
- `extractImageNames(const QString& xmlPath) -> QSet<QString>`：用正则 `img\d+\.jpg` 全文扫描 xml 提取所有图片名（小写，与文件系统一致）
- `compare(const QSet<QString>& folderNames, const QSet<QString>& xmlNames) -> struct DiffResult { QStringList onlyInFolder; QStringList onlyInXml; }`：集合差集
**Why**: 校对逻辑高内聚封装，正则方案与标签名解耦（兼容 `<imglink>` 和 `<图片链接>`）
**How**:
- 读 xml 全文为 QString，用 `QRegularExpression` 全局匹配 `img\d+\.jpg`，收集到 QSet
- 差集：`onlyInFolder = folderNames - xmlNames`，`onlyInXml = xmlNames - folderNames`
- 两个列表各自排序（按文件名字符串序）便于展示

### 新增文件 3: `d:\pinjie\src\CheckResultDialog.h` / `CheckResultDialog.cpp`
**What**: QDialog 子类，展示校对结果
**Why**: 差异可能几十上百条，QMessageBox 文本过长不人体工学；自定义 Dialog 双栏 QListWidget 清晰且支持滚动选择
**How**:
- 布局：QHBoxLayout 含两个 QLabel+QListWidget 组（左「只在 XML 中（N 条）」、右「只在图片库中（N 条）」），底部 QPushButton「关闭」
- 构造函数接收 `const QStringList& onlyInXml, const QStringList& onlyInFolder`
- 两个 QListWidget 均设 `setSelectionMode(SingleSelection)`、`setSortingEnabled(true)`、最小尺寸 300x400
- 两侧条目数为 0 时显示「无差异」占位文案
- 极简风格继承主窗口 stylesheet（白底、灰边、蓝按钮）

### 修改文件 1: `d:\pinjie\src\FileManager.h` / `FileManager.cpp`
**What**: 新增静态方法 `listJpgNames(const QString& folder) -> QSet<QString>`
**Why**: 扫描图片库 jpg 文件名，供校对使用
**How**:
- `QDir dir(folder); auto files = dir.entryList(QStringList() << "*.jpg" << "*.JPG", QDir::Files)`
- 逐个加入 QSet（小写化保证与 xml 提取的名称一致）
- 文件夹不存在返回空集合

### 修改文件 2: `d:\pinjie\src\MainWindow.h`
**What**: 新增成员和槽
- `QPushButton *m_smartImportBtn;`（智能导入按钮）
- `QPushButton *m_checkBtn;`（校对按钮）
- 槽 `void onSmartImport();`、`void onCheck();`
**Why**: UI 控件声明与槽函数绑定
**How**: 在现有 private 成员区追加，前置声明已含 QPushButton

### 修改文件 3: `d:\pinjie\src\MainWindow.cpp`
**What**: 构造函数新增两个按钮 + 实现两个槽
**Why**: 接线 UI 与逻辑
**How**:
- 构造函数：在 folderLabel 之前（最顶部）新增「智能导入」按钮，点击 connect 到 onSmartImport；在 m_outputBtn 之后（stretch 之前）新增「校对图片」按钮，connect 到 onCheck
- `onSmartImport()`：弹 `QFileDialog::getExistingDirectory` 选根目录 → 调 `PathResolver::resolveImageCutAndXml` → 成功则 `m_folderEdit->setText(imageCutDir)` + `m_xmlEdit->setText(xmlPath)`；失败弹 `QMessageBox::warning` 提示未找到 imageCut 或 xml
- `onCheck()`：读取 folderEdit 和 xmlEdit 路径 → 校验非空且存在 → `FileManager::listJpgNames(folder)` + `XmlChecker::extractImageNames(xmlPath)` → `XmlChecker::compare` → 构造 `CheckResultDialog` 并 exec()
- 新增 include：PathResolver.h、XmlChecker.h、CheckResultDialog.h、QSet、QDialog

### 修改文件 4: `d:\pinjie\src\CMakeLists.txt`
**What**: 在 `add_executable` 源文件列表追加 6 个新文件
**Why**: 让 CMake 编译新类
**How**: 追加 `PathResolver.h PathResolver.cpp XmlChecker.h XmlChecker.cpp CheckResultDialog.h CheckResultDialog.cpp`

---

## Assumptions & Decisions

1. **保留原有两个「浏览...」按钮**：智能导入是新增便捷功能，不替换手动浏览（处理非标准目录结构或多个 xml 需手动选的场景）。若用户希望替换，可在审阅时提出。
2. **正则全文扫描提取 xml 图片名**：基于 Phase 1 Grep 验证（806 次精准匹配 0 误匹配），与标签名解耦，兼容 `<imglink>` 和 `<图片链接>`。不依赖 QDomDocument 按标签提取，避免标签名硬编码。
3. **校对结果用自定义 QDialog**：差异条目可能很多，双栏 QListWidget 比 QMessageBox 富文本更人体工学。代码量可控（~40 行 Dialog 类）。
4. **智能导入按钮放面板顶部**：作为最高频入口放最上方，符合人体工学（用户进入软件第一操作）。校对按钮放输出按钮之后，作为辅助工具。
5. **文件名大小写处理**：jpg 文件名统一小写化后比较（xml 中 `img00001001.jpg` 与文件系统 `img00001001.jpg` 一致）。范例均为小写，但小写化防御性处理。
6. **不改动 XmlEditor/Stitcher/NumberUtil**：本次功能与拼接/输出流程正交，不影响现有逻辑。注意：现有 XmlEditor 对新范例 xml（英文标签）不兼容，但本次不修复（用户未要求，YAGNI）。
7. **递归查找 imageCut 找第一个即停**：若目录树中有多个 imageCut，取第一个找到的。范例结构只有一个，符合预期。
8. **xml 查找找第一个即停**：若目录树中有多个 xml，取第一个。范例只有一个目标 xml。

---

## Verification Steps

1. **编译验证**：`cmake --build d:\pinjie\src\build -j` 0 错误
2. **打包验证**：`windeployqt ImageStitcher.exe` 刷新依赖，启动无缺 DLL
3. **智能导入功能验证**：
   - 启动 exe，点「智能导入」，选 `C:\Users\E-VR\Desktop\06\C语言程序设计案例教程97871133186110000（1-1）`
   - 预期：folderEdit 填充为 `...\LSF\XML\imageCut`，xmlEdit 填充为 `...\LSF\XML\C语言程序设计案例教程97871133186110000（1-1）.xml`
   - 边界：选无 imageCut 的目录，弹警告提示未找到
4. **校对功能验证**：
   - 智能导入后点「校对图片」
   - 预期：弹出 Dialog 双栏展示差异。范例 xml 806 引用 vs imageCut 文件数（约 280），「只在 XML 中」应有约 526 条（xml 引用但图片库缺失），「只在图片库中」应为 0 或少量
   - 边界：无差异时两栏均显示「无差异」占位
5. **现有功能回归**：手动浏览按钮、预览、输出、备份、XML 开关、拉伸对齐均不受影响
6. **8 种组合预览**：拉伸对齐等 8 种组合仍正常（本次改动不涉及 Stitcher）
