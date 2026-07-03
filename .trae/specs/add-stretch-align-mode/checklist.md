# Checklist

## AlignMode 枚举
- [x] `Stitcher.h` 的 `AlignMode` 枚举新增 `Stretch` 值（在 Right 之后）
- [x] 枚举注释说明拉伸语义（小图等比放缩至拼接边匹配大图）

## Stitcher::stitch() Stretch 分支
- [x] Stretch 分支在统一格式转换之后、原 newW/newH 计算之前插入
- [x] 上下拼接：比较两图 width，较短者用 `scaledToWidth(较长者width, Qt::SmoothTransformation)` 放缩
- [x] 左右拼接：比较两图 height，较短者用 `scaledToHeight(较长者height, Qt::SmoothTransformation)` 放缩
- [x] 两图拼接边相等时不操作（if-else if 条件均不满足）
- [x] 放缩后画布尺寸：上下=放缩后宽 × 两图高之和；左右=两图宽之和 × 放缩后高
- [x] 绘制时无 offset（x=0 或 y=0）
- [x] Stretch 分支末尾 return canvas，不落入后续 offset 逻辑
- [x] 原 Left/Center/Right 逻辑在 Stretch 分支之后的路径保持不变

## MainWindow UI
- [x] 对齐方式 QHBoxLayout 新增第 4 个 RadioButton「拉伸」，id=3
- [x] 拉伸 RadioButton tooltip：「上下拼接：窄图等比放大至与宽图同宽；左右拼接：矮图等比放大至与高图同高」
- [x] 拉伸 RadioButton 加入 alignRow（rightRadio 之后、addStretch 之前）
- [x] 默认选中保持居左（stretchRadio 不 setChecked）

## MainWindow::currentAlign()
- [x] switch 新增 `case 3: return AlignMode::Stretch;`

## 编译与打包
- [x] `cmake --build` 0 错误（[100%] Built target ImageStitcher）
- [x] windeployqt 重新打包，DLL 齐全（均为 up to date）
- [x] ImageStitcher.exe 启动正常无缺 DLL

## 功能验证
- [x] 代码逻辑审查：8 种组合（2方向×4对齐）路径均有明确处理，无崩溃风险
- [x] 代码逻辑审查：拉伸上下拼接放缩窄图到宽图宽，左右拼接放缩矮图到高图高
- [x] 代码逻辑审查：两图拼接边相等时拉伸结果与居左一致（if-else if 不操作）
- [ ] 实际 UI 操作验证 8 种组合预览不崩溃、无空图 —— 待用户操作
- [ ] 实际 UI 操作验证拉伸后输出流程正常 —— 待用户操作

## 非功能
- [x] Win7 SP1+ 兼容性保持（Qt5.15，scaledToWidth/Height 早期 API）
- [x] 高内聚低耦合：拉伸逻辑集中在 Stitcher，UI 仅增 1 个 RadioButton + 1 个 switch case
- [x] 数据库 BC 范式：本项目无数据库，XML 结构无传递依赖
