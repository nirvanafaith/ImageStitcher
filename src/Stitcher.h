#pragma once
#include <QImage>
#include <QColor>

// 拼接方向
enum class StitchDirection {
    TopBottom,  // 上下拼接：img1 在上，img2 在下
    LeftRight   // 左右拼接：img1 在左，img2 在右
};

// 对齐模式（垂直于拼接方向的轴上的对齐；Stretch 例外：先等比放缩小图至拼接边匹配大图）
enum class AlignMode {
    Left,    // 上下拼接=窄图靠左；左右拼接=矮图靠上
    Center,  // 居中
    Right,   // 上下拼接=窄图靠右；左右拼接=矮图靠下
    Stretch  // 小图等比放缩至拼接边匹配大图，无偏移拼接
};

class Stitcher {
public:
    // 去除图片四周白色边缘，threshold 为判定白色的阈值（默认 245）
    static QImage trimWhiteBorders(const QImage& src, int threshold = 245);

    // 统计两张图片所有像素中出现次数最多的颜色（主色）。
    // 用于填充拼接画布留白区域。图片为空或全空时返回 Qt::white。
    static QColor dominantColor(const QImage& a, const QImage& b);

    // 拼接两张图片。
    // img1: 上下拼接时为上图、左右拼接时为左图
    // img2: 上下拼接时为下图、左右拼接时为右图
    // dir:  拼接方向
    // align: 对齐方式
    // trimBorders: 是否在拼接前去除两图四周白边
    // autoFillColor: 是否用两图主色自动填充画布留白（覆盖 fill）
    // fill: 画布空白填充色，默认白色（autoFillColor=true 时被忽略）
    // 返回拼接后的 QImage（Format_RGB32）；输入为空时返回空 QImage
    static QImage stitch(const QImage& img1, const QImage& img2,
                         StitchDirection dir, AlignMode align,
                         bool trimBorders = false,
                         bool autoFillColor = false,
                         QColor fill = Qt::white);
};
