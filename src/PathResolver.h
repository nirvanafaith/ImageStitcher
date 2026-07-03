#pragma once
#include <QString>

// 路径解析工具类（纯静态）：递归查找 imageCut 文件夹和第一个 xml 文件
class PathResolver {
public:
    // 从 root 向下递归查找名为 imageCut 的目录和第一个 .xml 文件。
    // 找到则分别写入 imageCutDir 和 xmlPath，返回 true。
    // 任一未找到返回 false（输出参数不被修改）。
    // 用 QDirIterator 增量遍历，找到即停，不遍历整棵树。
    static bool resolveImageCutAndXml(const QString& root,
                                      QString& imageCutDir, QString& xmlPath);
private:
    PathResolver() = delete;
};
