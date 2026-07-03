# UI 调整 + 图片名称补全功能计划

## Summary

1. **移动「同时修改 XML」复选框**：从右下角的预览区下方，移到左侧面板「去除白边」右侧，两者水平并排显示。
2. **图片名称补全按钮**：在「图片1名称」和「图片2名称」输入框右侧各加一个「补全」按钮。点击后把用户输入的 `63003`、`img63003` 或已有标准格式自动补全/规范为 `img00063003.jpg`。

---

## Current State Analysis（基于 Phase 1 探索）

### 现有 UI 布局（左侧面板，垂直排列）
1. 智能导入根目录
2. 图片文件夹（QLineEdit + 浏览）
3. XML 文件（QLineEdit + 浏览）
4. 图片1名称（QLineEdit，独占一行）
5. 图片2名称（QLineEdit，独占一行）
6. 去除白边（QCheckBox，独占一行）
7. 拼接方向（RadioButton 组）
8. 对齐方式（RadioButton 组）
9. 确认预览
10. 输出
11. 校对图片

右侧预览区下方有「同时修改 XML」复选框（`m_modifyXmlCheck`）。

### 图片命名规则（基于范例 `C:\Users\E-VR\Desktop\06\...\imageCut`）
- 所有文件名格式为 `img` + 8 位数字 + `.jpg`
- 例如 `img00063003.jpg`（数字部分 00063003）
- 数字部分固定 8 位，不足补前导零

### 关键约束
- Win7 SP1+ 兼容（Qt5.15.2 + MinGW，QHBoxLayout/QRegularExpression 为基础 API）
- 高内聚低耦合：补全算法集中在 `NumberUtil`，UI 只负责调用和设置文本
- 极简 UI：按钮文本简短、固定宽度、不破坏现有视觉一致性

---

## Proposed Changes

### 修改文件 1: `d:\pinjie\src\NumberUtil.h` / `d:\pinjie\src\NumberUtil.cpp`
**What**: 新增静态方法 `completeImageName(const QString& input) -> QString`。
**Why**: 把补全逻辑高内聚到数字/文件名处理工具类，UI 无感知规则细节。
**How**:
```cpp
// NumberUtil.h
static QString completeImageName(const QString& input);
```
```cpp
// NumberUtil.cpp
QString NumberUtil::completeImageName(const QString& input) {
    const QString s = input.trimmed();
    if (s.isEmpty()) return QString();

    // 已是标准格式 img000xxxxx.jpg，直接小写化返回
    static const QRegularExpression standardRe(
        QStringLiteral("^img\\d{8}\\.jpg$"),
        QRegularExpression::CaseInsensitiveOption);
    if (standardRe.match(s).hasMatch()) return s.toLower();

    // 提取连续数字
    static const QRegularExpression digitRe(QStringLiteral("(\\d+)"));
    const auto m = digitRe.match(s);
    if (!m.hasMatch()) return QString();

    bool ok = false;
    const quint64 num = m.captured(1).toULongLong(&ok);
    if (!ok) return QString();

    // 格式化为 img + 至少8位数字（前导零）+ .jpg
    return QStringLiteral("img%1.jpg").arg(num, 8, 10, QLatin1Char('0'));
}
```
注意：需要在 `NumberUtil.cpp` 顶部 include `<QRegularExpression>`。

### 修改文件 2: `d:\pinjie\src\MainWindow.h`
**What**: 新增补全按钮成员和槽函数。
**Why**: UI 控件声明与信号槽绑定。
**How**: 在私有成员区追加：
```cpp
    QPushButton *m_completeImg1Btn;  // 图片1名称补全按钮
    QPushButton *m_completeImg2Btn;  // 图片2名称补全按钮
```
在 `private slots` 区追加：
```cpp
    void onCompleteImg1();  // 补全图片1名称
    void onCompleteImg2();  // 补全图片2名称
```

### 修改文件 3: `d:\pinjie\src\MainWindow.cpp`

#### 改动 A: 图片1名称行改为 QHBoxLayout（输入框 + 补全按钮）
当前代码：
```cpp
    // 3. 图片1名称
    QLabel *img1Label = new QLabel(QStringLiteral("图片1名称(上/左)："), panel);
    panelLayout->addWidget(img1Label);
    m_img1Edit = new QLineEdit(panel);
    m_img1Edit->setPlaceholderText(QStringLiteral("img00020002.jpg"));
    panelLayout->addWidget(m_img1Edit);
```
改为：
```cpp
    // 3. 图片1名称
    QLabel *img1Label = new QLabel(QStringLiteral("图片1名称(上/左)："), panel);
    panelLayout->addWidget(img1Label);
    QHBoxLayout *img1Row = new QHBoxLayout();
    img1Row->setSpacing(6);
    m_img1Edit = new QLineEdit(panel);
    m_img1Edit->setPlaceholderText(QStringLiteral("img00020002.jpg"));
    m_completeImg1Btn = new QPushButton(QStringLiteral("补全"), panel);
    m_completeImg1Btn->setToolTip(QStringLiteral("将输入补全为 img000xxxxx.jpg 格式"));
    m_completeImg1Btn->setFixedWidth(50);
    connect(m_completeImg1Btn, &QPushButton::clicked, this, &MainWindow::onCompleteImg1);
    img1Row->addWidget(m_img1Edit);
    img1Row->addWidget(m_completeImg1Btn);
    panelLayout->addLayout(img1Row);
```

#### 改动 B: 图片2名称行改为 QHBoxLayout（输入框 + 补全按钮）
当前代码：
```cpp
    // 4. 图片2名称
    QLabel *img2Label = new QLabel(QStringLiteral("图片2名称(下/右)："), panel);
    panelLayout->addWidget(img2Label);
    m_img2Edit = new QLineEdit(panel);
    m_img2Edit->setPlaceholderText(QStringLiteral("img00020003.jpg"));
    panelLayout->addWidget(m_img2Edit);
```
改为：
```cpp
    // 4. 图片2名称
    QLabel *img2Label = new QLabel(QStringLiteral("图片2名称(下/右)："), panel);
    panelLayout->addWidget(img2Label);
    QHBoxLayout *img2Row = new QHBoxLayout();
    img2Row->setSpacing(6);
    m_img2Edit = new QLineEdit(panel);
    m_img2Edit->setPlaceholderText(QStringLiteral("img00020003.jpg"));
    m_completeImg2Btn = new QPushButton(QStringLiteral("补全"), panel);
    m_completeImg2Btn->setToolTip(QStringLiteral("将输入补全为 img000xxxxx.jpg 格式"));
    m_completeImg2Btn->setFixedWidth(50);
    connect(m_completeImg2Btn, &QPushButton::clicked, this, &MainWindow::onCompleteImg2);
    img2Row->addWidget(m_img2Edit);
    img2Row->addWidget(m_completeImg2Btn);
    panelLayout->addLayout(img2Row);
```

#### 改动 C: 「去除白边」独占一行改为与「同时修改 XML」水平并排
当前代码：
```cpp
    // 5. 去除白边选项
    m_trimBordersCheck = new QCheckBox(QStringLiteral("去除白边"), panel);
    m_trimBordersCheck->setChecked(false);
    m_trimBordersCheck->setToolTip(QStringLiteral("勾选后先去除两图四周白色边缘再拼接"));
    panelLayout->addWidget(m_trimBordersCheck);
```
改为：
```cpp
    // 5. 去除白边 + 同时修改 XML（水平并排）
    QHBoxLayout *optionRow = new QHBoxLayout();
    optionRow->setSpacing(12);
    m_trimBordersCheck = new QCheckBox(QStringLiteral("去除白边"), panel);
    m_trimBordersCheck->setChecked(false);
    m_trimBordersCheck->setToolTip(QStringLiteral("勾选后先去除两图四周白色边缘再拼接"));
    m_modifyXmlCheck = new QCheckBox(QStringLiteral("同时修改 XML"), panel);
    m_modifyXmlCheck->setChecked(true);
    m_modifyXmlCheck->setToolTip(QStringLiteral(
        "勾选：输出时同步修改源 XML（更新小序号图宽高、删除大序号图段落）\n"
        "不勾选：仅更新图片文件，不修改源 XML"));
    optionRow->addWidget(m_trimBordersCheck);
    optionRow->addWidget(m_modifyXmlCheck);
    optionRow->addStretch();
    panelLayout->addLayout(optionRow);
```

#### 改动 D: 移除右侧面板中的「同时修改 XML」复选框
当前右侧布局（约第 188-198 行）：
```cpp
    // 右下角"同时修改 XML"复选框
    QHBoxLayout *optionRow = new QHBoxLayout();
    optionRow->setContentsMargins(0, 0, 0, 0);
    optionRow->addStretch();
    m_modifyXmlCheck = new QCheckBox(QStringLiteral("同时修改 XML"), rightPanel);
    ...
    optionRow->addWidget(m_modifyXmlCheck);
    rightLayout->addLayout(optionRow);
```
整段删除。删除后右侧面板只剩 `m_displayLabel`，布局仍正常。

#### 改动 E: 实现两个补全槽函数
在 MainWindow.cpp 文件末尾（现有槽函数之后）追加：
```cpp
void MainWindow::onCompleteImg1()
{
    const QString text = NumberUtil::completeImageName(m_img1Edit->text());
    if (!text.isEmpty()) {
        m_img1Edit->setText(text);
    }
}

void MainWindow::onCompleteImg2()
{
    const QString text = NumberUtil::completeImageName(m_img2Edit->text());
    if (!text.isEmpty()) {
        m_img2Edit->setText(text);
    }
}
```
注意：`NumberUtil.h` 已在 `MainWindow.cpp` 顶部 include（现有代码已 include）。

---

## Assumptions & Decisions

1. **补全规则（兼容已有前缀）**：
   - `63003` → `img00063003.jpg`
   - `img63003` → `img00063003.jpg`
   - `img00063003.jpg` → `img00063003.jpg`（不变，仅小写化）
   - 输入不含任何数字则不操作。
2. **数字部分最小 8 位**：使用 `QString::arg(num, 8, 10, QLatin1Char('0'))`，若数字本身超过 8 位则按实际位数显示（不会截断）。
3. **补全按钮宽度固定 50px**：避免按钮随文本变化宽度，保持输入框空间稳定。
4. **XML 复选框与去除白边水平并排**：使用同一 QHBoxLayout，addStretch 让两者靠左自然分布。
5. **不改动其他文件**：Stitcher/FileManager/XmlEditor/PathResolver/XmlChecker/CheckResultDialog/CMakeLists.txt 均不受影响。
6. **数据库 BC 范式**：本项目无数据库，XML 结构无传递依赖。

---

## Verification Steps

1. **编译验证**：`cmake --build d:\pinjie\src\build -j` 0 错误
2. **打包验证**：`windeployqt ImageStitcher.exe` 刷新依赖，启动无缺 DLL
3. **UI 验证**：
   - 左侧面板「图片1名称」和「图片2名称」输入框右侧各有一个「补全」按钮
   - 「去除白边」和「同时修改 XML」在同一行水平并排
   - 右下角不再显示「同时修改 XML」复选框
4. **补全功能验证**：
   - 图片1输入 `63003` → 点击补全 → `img00063003.jpg`
   - 图片2输入 `img63003` → 点击补全 → `img00063003.jpg`
   - 图片1输入 `img00063003.jpg` → 点击补全 → `img00063003.jpg`（不变）
   - 输入无数字（如 `abc`）→ 点击补全无反应
5. **回归验证**：
   - 预览/输出流程正常
   - 同时修改 XML 勾选/不勾选状态仍正确影响输出行为
   - 去除白边勾选/不勾选仍正常
