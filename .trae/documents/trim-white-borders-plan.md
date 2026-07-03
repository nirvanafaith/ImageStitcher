# 去除白边功能计划

## Summary

在 MainWindow 左侧面板的「拼接方向」之前新增一个方形勾选框「去除白边」。勾选后，点击「确认预览」或「输出」时会先把两张待拼接图片的四周边缘白色区域裁剪掉，再交给拼接算法；不勾选则保持原行为不变。

实现方式：在 `Stitcher` 类中新增 `trimWhiteBorders()` 静态方法，并给 `stitch()` 增加一个 `bool trimBorders = false` 参数，由 UI 勾选框控制是否先裁剪。

---

## Current State Analysis（基于 Phase 1 探索）

### 现有代码结构
- [Stitcher.h](file:///d:/pinjie/src/Stitcher.h)：声明 `AlignMode` 枚举和 `stitch(img1, img2, dir, align, fill)`
- [Stitcher.cpp](file:///d:/pinjie/src/Stitcher.cpp)：实现 `stitch()`，先统一转 RGB32，再做拉伸/普通拼接。当前 `stitch()` 参数签名：
  ```cpp
  static QImage stitch(const QImage& img1, const QImage& img2,
                       StitchDirection dir, AlignMode align,
                       QColor fill = Qt::white);
  ```
- [MainWindow.h](file:///d:/pinjie/src/MainWindow.h)：私有成员含方向/对齐 RadioButton 组、预览/输出按钮等
- [MainWindow.cpp](file:///d:/pinjie/src/MainWindow.cpp)：左侧面板布局顺序为：图片文件夹 → XML 文件 → 图片1名称 → 图片2名称 → **拼接方向** → 对齐方式 → 确认预览 → 输出 → 校对图片
- `onPreview()` 是 `Stitcher::stitch()` 的唯一调用方；`onOutput()` 使用 `onPreview()` 缓存的 `m_stitchedCache`，不直接调用 `stitch()`

### 关键约束
- Win7 SP1+ 兼容（Qt5.15.2 + MinGW，QImage 像素遍历为基础 API）
- 高内聚低耦合：图像处理逻辑集中在 `Stitcher`，UI 只增加复选框并传递布尔参数
- 默认行为不变：勾选框默认不勾选，不破坏现有任何功能
- 性能：去边算法 O(w×h) 像素扫描，单张图 < 10ms，主线程可接受

---

## Proposed Changes

### 修改文件 1: `d:\pinjie\src\Stitcher.h`
**What**: 新增 `trimWhiteBorders()` 声明；修改 `stitch()` 签名，增加 `bool trimBorders = false` 参数。
**Why**: 把裁剪白边逻辑高内聚到图像处理类；`trimBorders` 参数让调用方决定是否先裁剪。
**How**:
```cpp
// 去除图片四周白色边缘，threshold 为判定白色的阈值（默认 245）
static QImage trimWhiteBorders(const QImage& src, int threshold = 245);

// stitch 新增 trimBorders 参数，默认 false 保持原行为
static QImage stitch(const QImage& img1, const QImage& img2,
                     StitchDirection dir, AlignMode align,
                     bool trimBorders = false, QColor fill = Qt::white);
```

### 修改文件 2: `d:\pinjie\src\Stitcher.cpp`
**What**: 实现 `trimWhiteBorders()`；在 `stitch()` 开头根据 `trimBorders` 决定是否先裁剪两图。
**Why**: 算法核心集中在本文件，UI 无感知细节。
**How**:
- `trimWhiteBorders()`：
  1. 输入为空返回空图；非 RGB32 则 `convertToFormat(QImage::Format_RGB32)`。
  2. 通过 `QImage::constBits()` 获取像素数据，按行/列扫描找到非白色像素的最小外接矩形。
  3. 白色判定：`qRed(c) >= threshold && qGreen(c) >= threshold && qBlue(c) >= threshold`。
  4. 若全图皆白，返回原图副本（避免宽/高为 0 导致后续崩溃）。
  5. 否则 `QImage::copy(left, top, width, height)` 返回裁剪结果。
- `stitch()` 内：
  ```cpp
  QImage a = img1.convertToFormat(QImage::Format_RGB32);
  QImage b = img2.convertToFormat(QImage::Format_RGB32);
  if (trimBorders) {
      a = trimWhiteBorders(a);
      b = trimWhiteBorders(b);
  }
  ```
  后续原拼接逻辑保持不变（包括 Stretch 分支）。

### 修改文件 3: `d:\pinjie\src\MainWindow.h`
**What**: 新增 `QCheckBox *m_trimBordersCheck;` 成员。
**Why**: 保存勾选框状态供 `onPreview()` 读取。
**How**: 在私有成员区合适位置追加，前置声明已含 `QCheckBox`。

### 修改文件 4: `d:\pinjie\src\MainWindow.cpp`
**What**: 构造函数添加复选框；`onPreview()` 调用 `stitch()` 时传入勾选状态。
**Why**: UI 控制开关，预览/输出复用同一缓存结果。
**How**:
- 构造函数：在 `// 5. 拼接方向` 之前插入：
  ```cpp
  // 5. 去除白边选项
  m_trimBordersCheck = new QCheckBox(QStringLiteral("去除白边"), panel);
  m_trimBordersCheck->setChecked(false);  // 默认不勾选，保持原行为
  m_trimBordersCheck->setToolTip(QStringLiteral("勾选后先去除两图四周白色边缘再拼接"));
  panelLayout->addWidget(m_trimBordersCheck);
  ```
- 后续注释数字仅做说明性调整（"// 5. 拼接方向"改为"// 6. 拼接方向"等），不影响功能。
- `onPreview()` 中：
  ```cpp
  m_stitchedCache = Stitcher::stitch(img1, img2, dir, align,
                                     m_trimBordersCheck->isChecked());
  ```
- `onOutput()` 直接使用 `m_stitchedCache`，无需改动。

---

## Assumptions & Decisions

1. **默认阈值 245**：白色背景常因 JPG 压缩/扫描有轻微噪点，245 是经验阈值，比严格 255 更鲁棒。用户未要求可调，YAGNI 不加滑块/输入框。
2. **白色判定基于 RGB 三通道同时 ≥ threshold**：与 Qt::white 一致，不单独处理 alpha（图片已转 RGB32，alpha 忽略）。
3. **全白图片返回原图副本**：避免裁剪后宽/高为 0 导致后续拼接画布创建失败或崩溃。
4. **只在 onPreview 处裁剪一次**：`onOutput` 复用 `m_stitchedCache`，避免重复计算且保证预览和输出结果一致。
5. **不裁剪缓存原图**：`trimWhiteBorders()` 返回副本，不影响输入图片文件。
6. **默认参数从右到左连续**：`stitch()` 新参数 `bool trimBorders = false` 必须放在 `fill` 之前，保证已有调用 `stitch(img1,img2,dir,align)` 仍合法。
7. **不改变 UI 其他元素位置**：复选框加在「拼接方向」之前，符合用户要求；不影响后续 stretch/校对按钮。
8. **数据库 BC 范式**：本项目无数据库，XML 结构无传递依赖，功能不涉及 XML 结构改动。

---

## Verification Steps

1. **编译验证**：`cmake --build d:\pinjie\src\build -j` 0 错误
2. **打包验证**：`windeployqt ImageStitcher.exe` 刷新依赖，启动无缺 DLL
3. **UI 验证**：启动 exe，在「图片2名称」和「拼接方向」之间能看到「去除白边」复选框，默认不勾选
4. **功能验证**：
   - 选两张带明显白边的测试图，不勾选时预览包含白边
   - 勾选后再次点预览，白边被去除，有效内容区域放大/靠拢
   - 切换拉伸/非拉伸 + 去白边组合，8 种组合仍正常
5. **输出验证**：勾选去白边后预览 → 输出，生成的覆盖图已去除白边
6. **回归验证**：不勾选去白边时，原有 8 种组合预览/输出行为与改动前完全一致
