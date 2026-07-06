# ImageStitcher 图片拼接工具

一款基于 Qt5 的图片拼接工具，专为出版社/排版工作流设计。将两张相邻图片拼接为一张，并同步修改源 XML 文件（更新宽高、删除多余段落）。支持上下/左右拼接、4 种对齐模式、去除白边、主色填充、智能导入、图片校对等功能。

兼容 Windows 7 SP1 及以上系统，免安装绿色版，解压即用。

## 功能特性

- **拼接方向**：上下拼接（img1 在上）/ 左右拼接（img1 在左）
- **对齐方式**：居左 / 中线 / 居右 / 拉伸
  - 拉伸模式：小图等比放缩至拼接边匹配大图，无偏移拼接
- **去除白边**：拼接前裁剪两图四周白色边缘（阈值 245，RGB 三通道均 ≥ 245 视为白色）
- **填充颜色**：用两图主色（QHash 像素直方图统计出现次数最多的颜色）填充画布留白区域
- **同时修改 XML**：输出时同步更新源 XML
  - 更新小序号图的 `<图片宽度>` 和 `<图片高度>`
  - 删除大序号图所在的整个 `<段落>` 块
- **智能导入**：选择根目录后自动递归查找 `imageCut` 文件夹和第一个 XML 文件
- **校对图片**：比较图片库与 XML 引用的图片名差异，列出 `onlyInFolder`（图片库有但 XML 没引用）和 `onlyInXml`（XML 引用但图片库没有）
- **序号补全**：输入 `63003` → `img00063003.jpg`（自动补全为 `img000xxxxx.jpg` 格式）
- **pageCode 一致性校验**：两图 pageCode 不一致时提示用户确认
- **自动备份**：输出前自动备份原图到 `原文件夹_backup` 目录
- **并行加载**：QtConcurrent::run 并行加载两张图片
- **Win7 SP1 兼容**：`_WIN32_WINNT=0x0601`，`-mwindows` 隐藏控制台窗口

## 下载与使用

### 下载

前往 [Releases 页面](https://github.com/nirvanafaith/ImageStitcher/releases) 下载最新版本：

- **ImageStitcher-v1.0-win7-x64.zip** - 解压后直接运行，免安装

### 系统要求

- Windows 7 SP1 及以上（64 位）
- 无需安装任何运行时，Qt5 和 MinGW 运行库已打包在内

### 使用流程

```
┌─────────────────────────────────────────────────────────────┐
│  ImageStitcher                                              │
├──────────────────────┬──────────────────────────────────────┤
│ 智能导入根目录       │                                      │
│ 图片文件夹：[____]   │                                      │
│ XML 文件：    [____] │           预览区域                   │
│ 图片1名称：  [____]  │                                      │
│ 图片2名称：  [____]  │                                      │
│ □去除白边 □同时修改XML □填充颜色                          │
│ 拼接方向： 上下/左右 │                                      │
│ 对齐方式： 居左/中线/居右/拉伸                             │
│ [确认预览] [输出]    │                                      │
│ [校对图片]           │                                      │
└──────────────────────┴──────────────────────────────────────┘
```

1. **导入路径**：点击「智能导入根目录」选择项目根目录，软件自动向下查找 `imageCut` 文件夹和第一个 XML 文件；或手动点击「浏览...」分别选择图片文件夹和 XML 文件
2. **输入图片名称**：在「图片1名称」和「图片2名称」中输入要拼接的两张图片文件名（如 `img00020002.jpg`）。可点击「补全」按钮将简写（如 `63003`）补全为完整格式
3. **选择拼接选项**：
   - 拼接方向：上下拼接 或 左右拼接
   - 对齐方式：居左 / 中线 / 居右 / 拉伸
   - 勾选「去除白边」：拼接前裁剪两图四周白色边缘
   - 勾选「填充颜色」：用两图主色填充画布留白（默认白色填充）
   - 勾选「同时修改 XML」：输出时同步修改源 XML 文件
4. **确认预览**：点击「确认预览」查看拼接效果，右侧预览区显示拼接结果
5. **输出**：确认无误后点击「输出」按钮，软件将：
   - 备份原图到 `原文件夹_backup` 目录
   - 用拼接图覆盖小序号图
   - 删除大序号图
   - （若勾选）修改源 XML
6. **校对图片**：点击「校对图片」比较图片库与 XML 引用差异，弹出结果对话框

## 拼接算法说明

### 画布尺寸计算

- **上下拼接**：宽度 = max(图1宽, 图2宽)，高度 = 图1高 + 图2高
- **左右拼接**：宽度 = 图1宽 + 图2宽，高度 = max(图1高, 图2高)

### 对齐偏移逻辑

对于非拉伸模式，窄图（或矮图）在拼接方向上的偏移：

| 对齐模式 | 偏移量 |
|----------|--------|
| 居左 | `0` |
| 中线 | `(total - size) / 2` |
| 居右 | `total - size` |

### 拉伸模式

拉伸模式下，小图等比放缩至拼接边匹配大图：
- 上下拼接：窄图 `scaledToWidth(宽图宽度)`
- 左右拼接：矮图 `scaledToHeight(高图高度)`

放缩后两图拼接边相等，无偏移拼接。

### 去除白边算法

四方向扫描裁剪：
1. 从上往下扫描，找到第一行非全白像素作为 `top`
2. 从下往上扫描，找到最后一行非全白像素作为 `bottom`
3. 从左往右扫描（在 top~bottom 范围内），找到第一列非全白像素作为 `left`
4. 从右往左扫描，找到最后一列非全白像素作为 `right`
5. 裁剪 `(left, top)` 到 `(right, bottom)` 的矩形区域

白色判定阈值默认 245：`qRed(c) >= 245 && qGreen(c) >= 245 && qBlue(c) >= 245`

### 填充颜色算法

用 `QHash<QRgb, quint64>` 统计两张图片所有像素的颜色直方图，返回出现次数最多的颜色作为主色填充画布留白区域。

## XML 同步修改说明

输出时若勾选「同时修改 XML」，软件会：

1. **更新小序号图宽高**：在 XML 中找到小序号图所在的 `<插图>`，更新其 `<图片宽度>` 为拼接图宽度、`<图片高度>` 为拼接图高度
2. **删除大序号图段落**：在 XML 中找到大序号图所在的整个 `<段落>` 块并删除
3. **pageCode 一致性校验**：预览时若两图 pageCode 不一致，提示用户确认是否继续

XML 采用原地覆盖写回，不产生新文件。

## 自动备份说明

输出前软件会自动备份两张原图到 `原文件夹_backup` 目录：
- 备份目录路径 = 原图片文件夹路径 + `_backup`
- 例：`D:/project/imageCut` → 备份到 `D:/project/imageCut_backup`
- 备份目录不存在则自动创建
- 目标已存在则先删除再复制（保证最新原图覆盖旧备份）
- 备份失败会终止输出操作，保护原图不被修改

## 开发环境

| 组件 | 版本 |
|------|------|
| 语言标准 | C++17 |
| Qt | 5.15.2（Widgets / Gui / Core / Xml / Concurrent） |
| 编译器 | MinGW 8.1.0 (64-bit) |
| 构建 | CMake 3.16+ |
| 目标平台 | Windows 7 SP1 及以上 |

## 从源码构建

### 环境准备

1. 安装 [Qt 5.15.2](https://download.qt.io/archive/qt/5.15/5.15.2/)（选择 MinGW 8.1.0 64-bit 版本）
2. 安装 [CMake 3.16+](https://cmake.org/download/)
3. 安装 MinGW 8.1.0 64-bit（Qt 安装包内含）

### 构建步骤

```bash
git clone https://github.com/nirvanafaith/ImageStitcher.git
cd ImageStitcher
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="<Qt安装路径>/5.15.2/mingw81_64" ..
cmake --build . --config Release
```

构建产物为 `ImageStitcher.exe`。

### Win7 兼容性

CMakeLists.txt 中已配置 Win7 兼容选项：

```cmake
# Win7 SP1 兼容（0x0601 = Windows 7）
if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0601 WINVER=0x0601 NOMINMAX)
endif()

# MinGW 隐藏控制台窗口
if(MINGW)
    target_link_options(ImageStitcher PRIVATE -mwindows)
endif()
```

## 项目结构

```
ImageStitcher/
├── src/                        # 源码目录
│   ├── main.cpp                # 程序入口
│   ├── MainWindow.h/.cpp       # 主窗口 UI（左侧控制面板 + 右侧预览区）
│   ├── Stitcher.h/.cpp         # 拼接算法（stitch/trimWhiteBorders/dominantColor）
│   ├── FileManager.h/.cpp      # 文件服务（加载/保存/备份/并行加载）
│   ├── XmlEditor.h/.cpp        # XML 原地修改（更新宽高+删段落）
│   ├── PathResolver.h/.cpp     # 智能导入（递归查找 imageCut 和 xml）
│   ├── XmlChecker.h/.cpp       # XML 图片名校对（比较图片库与 XML 引用差异）
│   ├── NumberUtil.h/.cpp       # 图片名序号工具（提取/补全 img000xxxxx.jpg）
│   ├── CheckResultDialog.h/.cpp # 校对结果对话框
│   └── CMakeLists.txt          # CMake 构建配置
├── output/                     # 已打包的发布版本
│   ├── ImageStitcher.exe       # 主程序
│   ├── Qt5*.dll                # Qt5 运行时
│   ├── platforms/qwindows.dll  # Qt 平台插件
│   ├── imageformats/*.dll      # Qt 图片格式插件
│   └── libgcc/libstdc++...     # MinGW 运行时
├── .trae/                      # 开发文档和设计规范
├── .gitignore
└── README.md
```

## 设计原则

- **高内聚低耦合**：每个模块职责单一，通过纯静态工具类组织，模块间无强依赖
- **极简 UI**：Fusion 风格，左侧控制面板 + 右侧预览区，操作流程线性清晰
- **性能优先**：并行加载图片、QHash 像素统计、scanLine 直接访问像素数据
- **安全第一**：输出前自动备份原图，备份失败则终止操作
- **Win7 兼容**：支持 Windows 7 SP1 及以上系统，无控制台窗口

## 许可证

MIT License

Copyright (c) 2026 nirvanafaith

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
