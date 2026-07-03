#include "Stitcher.h"
#include <QPainter>
#include <QHash>
#include <algorithm>

QImage Stitcher::stitch(const QImage& img1, const QImage& img2,
                        StitchDirection dir, AlignMode align,
                        bool trimBorders, bool autoFillColor, QColor fill)
{
    if (img1.isNull() || img2.isNull()) return QImage();

    // 统一格式为 RGB32
    QImage a = img1.convertToFormat(QImage::Format_RGB32);
    QImage b = img2.convertToFormat(QImage::Format_RGB32);

    // 去除白边：先裁剪两图四周白色边缘，再做后续拼接
    if (trimBorders) {
        a = trimWhiteBorders(a);
        b = trimWhiteBorders(b);
    }

    // 自动填充颜色：用两图主色替代固定 fill 色
    QColor actualFill = fill;
    if (autoFillColor) {
        actualFill = dominantColor(a, b);
    }

    // 拉伸对齐：先把小图等比放缩至拼接边匹配大图，再无 offset 拼接
    if (align == AlignMode::Stretch) {
        if (dir == StitchDirection::TopBottom) {
            // 拼接边=宽度，放缩窄图到宽图宽
            if (a.width() < b.width()) {
                a = a.scaledToWidth(b.width(), Qt::SmoothTransformation);
            } else if (b.width() < a.width()) {
                b = b.scaledToWidth(a.width(), Qt::SmoothTransformation);
            }
            // 两图现宽度相等
            const int w = a.width();
            const int h = a.height() + b.height();
            QImage canvas(w, h, QImage::Format_RGB32);
            canvas.fill(actualFill.rgb());
            QPainter p(&canvas);
            p.drawImage(0, 0, a);
            p.drawImage(0, a.height(), b);
            p.end();
            return canvas;
        } else {
            // 拼接边=高度，放缩矮图到高图高
            if (a.height() < b.height()) {
                a = a.scaledToHeight(b.height(), Qt::SmoothTransformation);
            } else if (b.height() < a.height()) {
                b = b.scaledToHeight(a.height(), Qt::SmoothTransformation);
            }
            // 两图现高度相等
            const int w = a.width() + b.width();
            const int h = a.height();
            QImage canvas(w, h, QImage::Format_RGB32);
            canvas.fill(actualFill.rgb());
            QPainter p(&canvas);
            p.drawImage(0, 0, a);
            p.drawImage(a.width(), 0, b);
            p.end();
            return canvas;
        }
    }

    int newW, newH;
    if (dir == StitchDirection::TopBottom) {
        newW = std::max(a.width(), b.width());
        newH = a.height() + b.height();
    } else { // LeftRight
        newW = a.width() + b.width();
        newH = std::max(a.height(), b.height());
    }

    QImage canvas(newW, newH, QImage::Format_RGB32);
    canvas.fill(actualFill.rgb());  // 填充背景色

    QPainter p(&canvas);
    // 根据对齐计算偏移的 lambda
    auto offset = [](int size, int total, AlignMode m) -> int {
        switch (m) {
        case AlignMode::Left:   return 0;
        case AlignMode::Center: return (total - size) / 2;
        case AlignMode::Right:  return total - size;
        }
        return 0;
    };

    if (dir == StitchDirection::TopBottom) {
        // 上下拼接：对齐作用于 x 轴
        int x1 = offset(a.width(), newW, align);
        int x2 = offset(b.width(), newW, align);
        p.drawImage(x1, 0, a);
        p.drawImage(x2, a.height(), b);
    } else {
        // 左右拼接：对齐作用于 y 轴
        int y1 = offset(a.height(), newH, align);
        int y2 = offset(b.height(), newH, align);
        p.drawImage(0, y1, a);
        p.drawImage(a.width(), y2, b);
    }
    p.end();
    return canvas;
}

QImage Stitcher::trimWhiteBorders(const QImage& src, int threshold)
{
    if (src.isNull()) return QImage();

    // 统一格式为 RGB32，便于 QRgb 访问
    const QImage img = src.convertToFormat(QImage::Format_RGB32);
    const int w = img.width();
    const int h = img.height();
    if (w <= 0 || h <= 0) return QImage();

    // 判定白色的 lambda：RGB 三通道均 >= threshold
    const auto isWhite = [threshold](QRgb c) -> bool {
        return qRed(c) >= threshold && qGreen(c) >= threshold && qBlue(c) >= threshold;
    };

    // 找上边界 top：从上往下扫描，遇到非白像素停止
    int top = 0;
    for (; top < h; ++top) {
        const QRgb *row = reinterpret_cast<const QRgb*>(img.scanLine(top));
        bool rowAllWhite = true;
        for (int x = 0; x < w; ++x) {
            if (!isWhite(row[x])) { rowAllWhite = false; break; }
        }
        if (!rowAllWhite) break;
    }
    if (top == h) return img.copy();  // 全白，返回原图副本避免 0 尺寸

    // 找下边界 bottom
    int bottom = h - 1;
    for (; bottom >= top; --bottom) {
        const QRgb *row = reinterpret_cast<const QRgb*>(img.scanLine(bottom));
        bool rowAllWhite = true;
        for (int x = 0; x < w; ++x) {
            if (!isWhite(row[x])) { rowAllWhite = false; break; }
        }
        if (!rowAllWhite) break;
    }

    // 找左边界 left
    int left = 0;
    for (; left < w; ++left) {
        bool colAllWhite = true;
        for (int y = top; y <= bottom; ++y) {
            const QRgb *row = reinterpret_cast<const QRgb*>(img.scanLine(y));
            if (!isWhite(row[left])) { colAllWhite = false; break; }
        }
        if (!colAllWhite) break;
    }

    // 找右边界 right
    int right = w - 1;
    for (; right >= left; --right) {
        bool colAllWhite = true;
        for (int y = top; y <= bottom; ++y) {
            const QRgb *row = reinterpret_cast<const QRgb*>(img.scanLine(y));
            if (!isWhite(row[right])) { colAllWhite = false; break; }
        }
        if (!colAllWhite) break;
    }

    // 拷贝有效区域
    const int cropW = right - left + 1;
    const int cropH = bottom - top + 1;
    if (cropW <= 0 || cropH <= 0) return img.copy();
    return img.copy(left, top, cropW, cropH);
}

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
