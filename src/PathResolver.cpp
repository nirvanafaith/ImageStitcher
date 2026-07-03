#include "PathResolver.h"
#include <QDirIterator>
#include <QDir>

bool PathResolver::resolveImageCutAndXml(const QString& root,
                                         QString& imageCutDir, QString& xmlPath)
{
    if (root.isEmpty() || !QDir(root).exists()) return false;

    // 1. 递归查找名为 imageCut 的目录（找到第一个即停）
    {
        QDirIterator it(root, QDir::Dirs | QDir::NoSymLinks, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            if (it.fileName() == QStringLiteral("imageCut")) {
                imageCutDir = it.filePath();
                break;
            }
        }
    }

    // 2. 递归查找第一个 .xml 文件（找到即停）
    {
        QDirIterator it(root, QStringList() << QStringLiteral("*.xml"),
                        QDir::Files, QDirIterator::Subdirectories);
        if (it.hasNext()) {
            it.next();
            xmlPath = it.filePath();
        }
    }

    return !imageCutDir.isEmpty() && !xmlPath.isEmpty();
}
