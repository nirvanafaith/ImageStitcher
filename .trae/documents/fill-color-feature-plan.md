# 填充颜色功能计划

## Summary

在左侧面板「同时修改 XML」复选框右侧新增一个「填充颜色」复选框。勾选后，拼接时因对齐方式（非 Stretch）产生的画布留白区域不再用默认白色填充，而是用「两张图所有像素中出现次数最多的颜色」（主色）填充。不勾选则保持原行为（白色填充）。

---

## Current State Analysis（基于 Phase 1 探索）

### 现有代码结构
- [Stitcher.h](file:///d:/pinjie/src/Stitcher.h)：声明 `stitch(img1, img2, dir, align, trimBorders=false, fill=Qt::white)` 和 `trimWhiteBorders`
- [Stitcher.cpp](file:///d:/pinjie/src/Stitcher.cpp)：`stitch()` 流程：转RGB32 → 可选 trimBorders → 可选 Stretch 放缩 → 创建 canvas 并 `canvas.fill(fill.rgb())` → drawImage 按 align offset 绘制
- [MainWindow.cpp 第 122-136 行](file:///d:/pinjie/src/MainWindow.cpp#L122-L136)：`optionRow` 水平布局含「去除白边」和「同时修改 XML」两个复选框
- [MainWindow.cpp 第 335-336 行](file:///d:/pinjie/src/MainWindow.cpp#L335-L336)：`onPreview()` 调用 `Stitcher::stitch(img1, img2, dir, align, m_trimBordersCheck->isChecked())`

### 留白产生场景
- 当 `align` 为 `Left/Center/Right` 且两图尺寸不同时，画布尺寸取 `max(w1,w2)` 或 `max(h1,h2)`，较小图无法覆盖的区域形成留白
- `Stretch` 模式下两图等比放缩至拼接边等长，无留白（功能对此模式无害，仍可勾选但不影响结果）

### 关键约束
- Win7 SP1+ 兼容（Qt5.15.2 + MinGW，QHash/QRgb 为基础 API）
- 高内聚低耦合：颜色统计逻辑集中在 `Stitcher`，UI 只增加复选框并传递布尔参数
- 默认行为不变：复选框默认不勾选
- 性能：主色统计 O(w×h)，单张图约 100-300ms，仅在勾选时计算，主线程可接受

---

## Proposed Changes

### 修改文件 1: `d:\pinjie\src\Stitcher.h`
**What**: 新增 `dominantColor()` 声明；`stitch()` 签名增加 `bool autoFillColor = false` 参数。
**Why**: 把主色统计逻辑高内聚到图像处理类；`autoFillColor` 参数让调用方决定是否自动填充。
**How**:
```cpp
class Stitcher {
public:
    static QImage trimWhiteBorders(const QImage& src, int threshold = 245);

    // 统计两张图片所有像素中出现次数最多的颜色（主色）。
    // 用于填充拼接画布留白区域。图片为空或全空时返回 Qt::white。
    static QColor dominantColor(const QImage& a, const QImage& b);

    static QImage stitch(const QImage& img1, const QImage& img2,
                         StitchDirection dir, AlignMode align,
                         bool trimBorders = false,
                         bool autoFillColor = false,
                         QColor fill = Qt::white);
};
```
注意：`autoFillColor` 必须放在 `fill` 之前，保证已有调用 `stitch(img1,img2,dir,align,trimBorders)` 仍合法。

### 修改文件 2: `d:\pinjie\src\Stitcher.cpp`
**What**: 实现 `dominantColor()`；在 `stitch()` 中根据 `autoFillColor` 决定是否用主色替代 `fill`。
**Why**: 算法核心集中在本文件。
**How**:

在文件顶部 include 区追加 `#include <QHash>`。

在 `stitch()` 内部，**在 trimBorders 和 Stretch 处理之后、创建 canvas 之前**（即第 58 行 Stretch 分支结束之后、第 60 行 `int newW, newH` 之前）插入：
```cpp
    // 自动填充颜色：用两图主色替代固定 fill 色
    QColor actualFill = fill;
    if (autoFillColor) {
        actualFill = dominantColor(a, b);
    }
```

然后把后续两处 `canvas.fill(fill.rgb())` 改为 `canvas.fill(actualFill.rgb())`：
- Stretch 分支内上下拼接的 `canvas.fill(fill.rgb())` → `canvas.fill(actualFill.rgb())`
- Stretch 分支内左右拼接的 `canvas.fill(fill.rgb())` → `canvas.fill(actualFill.rgb())`
- 非 Stretch 分支的 `canvas.fill(fill.rgb())` → `canvas.fill(actualFill.rgb())`

注意：Stretch 分支在 `autoFillColor` 计算之前就 return 了。为了让 Stretch 模式也支持填充色（虽然无留白但保持逻辑一致），需要把 `autoFillColor` 计算移到 Stretch 分支之前。修订实现：

在 `trimBorders` 处理之后、Stretch 分支之前插入：
```cpp
    // 自动填充颜色：用两图主色替代固定 fill 色
    QColor actualFill = fill;
    if (autoFillColor) {
        actualFill = dominantColor(a, b);
    }
```

然后把所有 `canvas.fill(fill.rgb())` 改为 `canvas.fill(actualFill.rgb())`（共 3 处：Stretch 上下、Stretch 左右、非 Stretch）。

在文件末尾追加 `dominantColor` 实现：
```cpp
QColor Stitcher::dominantColor(const QImage& a, const QImage& b)
{
    if (a.isNull() && b.isNull()) return Qt::white;

    QHash<QRgb, quint64> hist;
    auto countImage = [&](const QImage& img) {
        if (img.isNull()) return;
        for (int y = 0; y < img.height(); ++y) {
            const QRgb *row = reinterpret_cast<const QRgb*>(img.scanLine(y));
            for (int x = 0; x < img.width(); ++x) {
                ++hist[row[x]];
            }
        }
    };
    countImage(a);
    countImage(b);

    if (hist.isEmpty()) return Qt::white;

    QRgb bestRgb = 0xFFFFFFFF;
    quint64 bestCount = 0;
    for (auto it = hist.begin(); it != hist.end(); ++it) {
        if (it.value() > bestCount) {
            bestRgb = it.key();
            bestCount = it.value();
        }
    }
    return QColor(bestRgb);
}
```

### 修改文件 3: `d:\pinjie\src\MainWindow.h`
**What**: 新增 `QCheckBox *m_fillColorCheck;` 成员。
**Why**: 保存勾选框状态供 `onPreview()` 读取。
**How**: 在 `m_trimBordersCheck` 之后追加：
```cpp
    QCheckBox *m_fillColorCheck;  // 填充颜色勾选框
```

### 修改文件 4: `d:\pinjie\src\MainWindow.cpp`

#### 改动 A: optionRow 中在「同时修改 XML」右侧新增「填充颜色」复选框
当前代码（第 122-136 行）：
```cpp
    // 5. 去除白边 + 同时修改 XML（水平并排）
    QHBoxLayout *optionRow = new QHBoxLayout();
    optionRow->setSpacing(12);
    m_trimBordersCheck = new QCheckBox(QStringLiteral("去除白边"), panel);
    m_trimBordersCheck->setChecked(false);  // 默认不勾选，保持原行为
    m_trimBordersCheck->setToolTip(QStringLiteral("勾选后先去除两图四周白色边缘再拼接"));
    m_modifyXmlCheck = new QCheckBox(QStringLiteral("同时修改 XML"), panel);
    m_modifyXmlCheck->setChecked(true);  // 默认勾选，保持原行为
    m_modifyXmlCheck->setToolTip(QStringLiteral(
        "勾选：输出时同步修改源 XML（更新小序号图宽高、删除大序号图段落）\n"
        "不勾选：仅更新图片文件，不修改源 XML"));
    optionRow->addWidget(m_trimBordersCheck);
    optionRow->addWidget(m_modifyXmlCheck);
    optionRow->addStretch();
    panelLayout->addLayout(optionRow);
```
改为：
```cpp
    // 5. 去除白边 + 同时修改 XML + 填充颜色（水平并排）
    QHBoxLayout *optionRow = new QHBoxLayout();
    optionRow->setSpacing(12);
    m_trimBordersCheck = new QCheckBox(QStringLiteral("去除白边"), panel);
    m_trimBordersCheck->setChecked(false);  // 默认不勾选，保持原行为
    m_trimBordersCheck->setToolTip(QStringLiteral("勾选后先去除两图四周白色边缘再拼接"));
    m_modifyXmlCheck = new QCheckBox(QStringLiteral("同时修改 XML"), panel);
    m_modifyXmlCheck->setChecked(true);  // 默认勾选，保持原行为
    m_modifyXmlCheck->setToolTip(QStringLiteral(
        "勾选：输出时同步修改源 XML（更新小序号图宽高、删除大序号图段落）\n"
        "不勾选：仅更新图片文件，不修改源 XML"));
    m_fillColorCheck = new QCheckBox(QStringLiteral("填充颜色"), panel);
    m_fillColorCheck->setChecked(false);  // 默认不勾选，保持原行为
    m_fillColorCheck->setToolTip(QStringLiteral(
        "勾选后用两图主色（出现次数最多的像素颜色）填充拼接画布的留白区域\n"
        "不勾选则用白色填充"));
    optionRow->addWidget(m_trimBordersCheck);
    optionRow->addWidget(m_modifyXmlCheck);
    optionRow->addWidget(m_fillColorCheck);
    optionRow->addStretch();
    panelLayout->addLayout(optionRow);
```

#### 改动 B: onPreview() 调用 stitch 时传入 autoFillColor 状态
当前代码（第 335-336 行）：
```cpp
    m_stitchedCache = Stitcher::stitch(img1, img2, dir, align,
                                       m_trimBordersCheck->isChecked());
```
改为：
```cpp
    m_stitchedCache = Stitcher::stitch(img1, img2, dir, align,
                                       m_trimBordersCheck->isChecked(),
                                       m_fillColorCheck->isChecked());
```

---

## Assumptions & Decisions

1. **主色定义**：两张图所有像素中精确出现次数最多的 RGB 颜色。不做颜色聚类（YAGNI），JPG 压缩噪声可能导致颜色分散，但用户明确要求「像素颜色」。
2. **统计时机**：在 `trimBorders` 和 `Stretch` 之后计算主色，确保统计的是实际参与拼接的图片内容（而非原图）。
3. **性能**：QHash<QRgb, quint64> 对 600 万像素约 100-300ms，仅在勾选时计算，主线程可接受。如未来需优化可改用数组+量化，但当前 YAGNI。
4. **Stretch 模式**：无留白但仍计算主色（无害，保持逻辑统一，避免分支复杂化）。
5. **默认参数从右到左连续**：`autoFillColor` 放在 `fill` 之前，已有调用兼容。
6. **全空图片返回 Qt::white**：避免空直方图导致未定义行为。
7. **不改动其他文件**：FileManager/XmlEditor/PathResolver/XmlChecker/CheckResultDialog/NumberUtil/CMakeLists.txt 均不受影响。
8. **数据库 BC 范式**：本项目无数据库，XML 结构无传递依赖。

---

## Verification Steps

1. **编译验证**：`cmake --build d:\pinjie\src\build -j` 0 错误
2. **打包验证**：`windeployqt ImageStitcher.exe` 刷新依赖，启动无缺 DLL
3. **UI 验证**：左侧面板「同时修改 XML」右侧出现「填充颜色」复选框，默认不勾选
4. **功能验证**：
   - 选两张尺寸不同的图，对齐选「居中」（产生留白）
   - 不勾选「填充颜色」预览 → 留白为白色
   - 勾选「填充颜色」预览 → 留白变为两图主色
   - 切换对齐为「拉伸」+ 勾选「填充颜色」→ 无留白，结果与不勾选一致
5. **回归验证**：
   - 不勾选「填充颜色」时，所有原有功能（预览/输出/去白边/校对/智能导入）行为不变
   - 勾选「填充颜色」+ 输出 → 覆盖保存的图片留白为主色
