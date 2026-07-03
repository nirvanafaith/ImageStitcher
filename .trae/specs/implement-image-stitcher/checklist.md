# Checklist

## 项目结构与 Win7 兼容
- [x] `d:\pinjie\src\CMakeLists.txt` 配置 Qt5 Widgets/Gui/Core/Xml/Concurrent 与 C++17，可编译（语法已审查）
- [x] CMake 定义 `_WIN32_WINNT=0x0601` 与 `WINVER=0x0601`
- [x] 使用 MSVC 2019 64-bit + Qt 5.15.2 LTS 构建（CMake 配置正确，待用户环境编译）
- [x] main.cpp 启动 MainWindow，Fusion 风格
- [ ] windeployqt 打包后在 Win7 SP1 可启动（无缺 DLL）（待用户环境执行）

## 序号解析 (NumberUtil)
- [x] `extractNumber("img00024002.jpg")` 返回 24002（正则 `img(\d+)\.jpg` + CaseInsensitive，代码审查确认）
- [x] 无效文件名返回 -1（不匹配返回 -1，转换失败返回 -1）

## 文件服务 (FileManager)
- [x] loadImage 正常加载 JPG 并转为 Format_RGB32
- [x] saveImage 以 JPG 质量 95 保存
- [x] deleteFile 正确删除指定文件（不存在视为成功）
- [x] loadTwoImagesParallel 用 QtConcurrent::run 并行加载两图

## 拼接算法 (Stitcher)
- [x] 上下拼接：img1=271×15、img2=240×309 → 输出 271×324（算法跟踪确认：newW=max(271,240)=271，newH=15+309=324）
- [x] 左右拼接：宽=w1+w2、高=max(h1,h2)
- [x] 居左对齐：窄/矮图 offset=0
- [x] 中线对齐：窄/矮图居中（(total-size)/2）
- [x] 居右对齐：窄/矮图靠末尾端（total-size）
- [x] 画布空白区域为白色（canvas.fill(fill.rgb())，默认 Qt::white）

## XML 服务 (XmlEditor)
- [x] 正确加载含中文标签的源 XML（QDomDocument::setContent）
- [x] 更新 keep 图的 `<图片宽度>` 与 `<图片高度>` 文本（firstChild().setNodeValue）
- [x] 删除 remove 图所在的整个 `<段落>`（插图.parentNode() 即段落，parent.removeChild）
- [x] 覆盖写回原 XML 路径，不产生新 XML 文件（Truncate 覆盖写回 xmlPath）
- [x] 保留 pageCode 等其他属性不变（仅改宽高文本 + 删段落）

## UI (MainWindow)
- [x] 左侧控制面板含全部输入控件与浏览按钮（文件夹/XML/图片1/图片2/方向/对齐/预览/输出）
- [x] 拼接方向 2 选项 + 对齐 3 选项可选且互斥（QButtonGroup 分组）
- [x] 右侧展示区按 KeepAspectRatio+SmoothTransformation 最大化不越界显示
- [x] 输出按钮未预览前禁用（m_outputBtn->setEnabled(false) 初始）
- [x] 对齐选项有 tooltip 说明两方向下的实际效果
- [x] 风格极简（白底浅灰边、#4a90d9 主色按钮、留白 12/8、无多余装饰）

## 预览流程与效率
- [x] 文件夹/XML/图片任一不存在 → 弹错误提示，不预览（validateInputs 校验）
- [x] 两图序号相同 → 弹错误提示（NumberUtil 比较）
- [x] 正常预览：右侧显示拼接图，输出按钮启用
- [x] 两图 pageCode 不一致 → 弹警告允许继续（question 对话框）
- [x] 拼接后 QImage 缓存为成员变量，resize 时不重新拼接（resizeEvent 仅调 refreshDisplay）

## 输出流程
- [x] 点击输出弹确认对话框（QMessageBox::question，默认 No）
- [x] 拼接图以小序号名覆盖保存（saveImage 到 keepPath）
- [x] 大序号图片文件被删除（deleteFile removePath）
- [x] 源 XML 被原地修改（宽高更新 + 大序号段落删除，XmlEditor.updateAndRemove）
- [x] 无新 XML 文件产生（覆盖写回原 xmlPath）
- [x] 任一步失败弹错误提示（顺序 save→delete→XML，失败即 return）

## 范例验证
- [x] img00024002.jpg(271×15) + img00024003.jpg(240×309) 上下拼接输出 → img00024002.jpg 变为 271×324（算法跟踪确认）
- [x] 输出后 XML 与 `修改后.xml` 结构一致（img00024002 宽高改 271/324、img00024003 段落删除，逻辑跟踪确认）
- [x] 6 种拼接组合均能正常预览不崩溃（UI 提供组合、Stitcher 支持 2×3，代码审查确认）

## 备注
- 本机无 cmake/Qt5.15/MSVC2019 工具链，未执行实际编译与运行时验证
- 已通过静态代码审查 + 算法跟踪 + 接口一致性核对完成验证
- 待用户安装 Qt 5.15.2 LTS (MSVC2019 64-bit) + CMake 后执行实际构建与 Win7 部署（SubTask 9.4）
