# Tasks

- [x] Task 1: AlignMode 枚举新增 Stretch 值
  - [x] SubTask 1.1: 在 `Stitcher.h` 的 `AlignMode` 枚举末尾新增 `Stretch`（在 Right 之后），并更新枚举注释说明拉伸语义
- [x] Task 2: Stitcher::stitch() 新增 Stretch 分支
  - [x] SubTask 2.1: 在 `Stitcher.cpp` 的 stitch() 函数中，紧接统一格式转换之后、原 newW/newH 计算之前，插入 `if (align == AlignMode::Stretch)` 分支
  - [x] SubTask 2.2: Stretch 分支逻辑：上下拼接时比较 a.width() 与 b.width()，较短者用 `scaledToWidth(较长者width, Qt::SmoothTransformation)` 放缩；左右拼接时比较 a.height() 与 b.height()，较短者用 `scaledToHeight(较长者height, Qt::SmoothTransformation)` 放缩。两图拼接边相等则不操作
  - [x] SubTask 2.3: 放缩后计算画布尺寸：上下=w=放缩后宽、h=两图高之和；左右=w=两图宽之和、h=放缩后高
  - [x] SubTask 2.4: 创建画布并填充背景色，绘制 a 在 (0,0)，绘制 b 在 (0, a.height())（上下）或 (a.width(), 0)（左右），无 offset。p.end() 后 return canvas
  - [x] SubTask 2.5: 原 Left/Center/Right 逻辑保持不变（在 Stretch 分支 if 之外的后续路径）
- [x] Task 3: MainWindow UI 新增「拉伸」RadioButton
  - [x] SubTask 3.1: 在 `MainWindow.cpp` 构造函数对齐方式 QHBoxLayout 中，在 rightRadio 之后新增 `QRadioButton *stretchRadio = new QRadioButton(QStringLiteral("拉伸"), panel);`，加入 m_alignGroup id=3
  - [x] SubTask 3.2: 设置 stretchRadio 的 tooltip：「上下拼接：窄图等比放大至与宽图同宽；左右拼接：矮图等比放大至与高图同高」
  - [x] SubTask 3.3: 将 stretchRadio 加入 alignRow 布局（在 rightRadio 之后、addStretch 之前）
- [x] Task 4: MainWindow::currentAlign() 加 Stretch 映射
  - [x] SubTask 4.1: 在 `MainWindow.cpp` 的 currentAlign() switch 中新增 `case 3: return AlignMode::Stretch;`
- [x] Task 5: 重新编译 + windeployqt 打包
  - [x] SubTask 5.1: `cmake --build d:\pinjie\src\build -j` 重新编译，0 错误（[100%] Built target ImageStitcher）
  - [x] SubTask 5.2: 进入 build 目录执行 `windeployqt ImageStitcher.exe` 刷新依赖（DLL 均为 up to date）
  - [x] SubTask 5.3: 启动 ImageStitcher.exe 验证启动正常无缺 DLL（GUI 稳定运行）
- [x] Task 6: 8 种组合预览验证
  - [x] SubTask 6.1: 代码逻辑审查 2方向×4对齐=8 种组合路径均有明确处理，无未覆盖 case，无崩溃风险
  - [x] SubTask 6.2: Stretch 分支逻辑验证：上下放缩窄图到宽图宽、左右放缩矮图到高图高，拼接边相等时不操作（等同 Left）
  - [ ] SubTask 6.3: 实际 UI 操作验证 8 种组合预览（待用户在 GUI 中操作确认）

# Task Dependencies
- [Task 2] depends on [Task 1]
- [Task 3/4] depends on [Task 1]
- [Task 5] depends on [Task 2, Task 3, Task 4]
- [Task 6] depends on [Task 5]
- [Task 1/3/4] 可并行启动
