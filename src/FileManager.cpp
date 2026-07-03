#include "FileManager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QtConcurrent>

QImage FileManager::loadImage(const QString& folder, const QString& imgName) {
    const QString path = joinPath(folder, imgName);
    QImage img(path);
    if (img.isNull()) {
        return QImage();  // 返回空 QImage
    }
    return img.convertToFormat(QImage::Format_RGB32);
}

bool FileManager::saveImage(const QString& path, const QImage& image) {
    return image.save(path, "JPG", 95);
}

bool FileManager::deleteFile(const QString& path) {
    if (!QFile::exists(path)) {
        return true;  // 文件不存在视为成功
    }
    return QFile::remove(path);
}

QString FileManager::joinPath(const QString& folder, const QString& imgName) {
    return QDir(folder).filePath(imgName);
}

std::pair<QImage, QImage> FileManager::loadTwoImagesParallel(const QString& folder,
                                                             const QString& name1,
                                                             const QString& name2) {
    // 并行加载两张图片，再同步等待结果
    QFuture<QImage> f1 = QtConcurrent::run(loadImage, folder, name1);
    QFuture<QImage> f2 = QtConcurrent::run(loadImage, folder, name2);
    return { f1.result(), f2.result() };
}

QString FileManager::backupDirFor(const QString& folder) {
    // 父目录路径 + 文件夹名 + "_backup"
    const QFileInfo info(folder);
    return info.absolutePath() + '/' + info.fileName() + QStringLiteral("_backup");
}

bool FileManager::backupFile(const QString& srcPath, const QString& backupDir) {
    if (!QFile::exists(srcPath)) return true;  // 源不存在视为无需备份
    // 创建备份目录（含所有父目录）
    if (!QDir().mkpath(backupDir)) return false;
    const QString dstPath = QDir(backupDir).filePath(QFileInfo(srcPath).fileName());
    // 目标已存在先删除（覆盖旧备份，保证最新原图）
    if (QFile::exists(dstPath)) {
        if (!QFile::remove(dstPath)) return false;
    }
    return QFile::copy(srcPath, dstPath);
}

bool FileManager::backupFiles(const QString& folder, const QStringList& fileNames) {
    const QString backupDir = backupDirFor(folder);
    for (const QString& name : fileNames) {
        if (!backupFile(joinPath(folder, name), backupDir)) return false;
    }
    return true;
}

QSet<QString> FileManager::listJpgNames(const QString& folder) {
    QSet<QString> names;
    QDir dir(folder);
    if (!dir.exists()) return names;
    // 同时匹配大小写 .jpg 和 .JPG
    const QStringList files = dir.entryList(QStringList() << QStringLiteral("*.jpg") << QStringLiteral("*.JPG"),
                                            QDir::Files);
    for (const QString& f : files) {
        names.insert(f.toLower());  // 小写化保证与 xml 提取的名称一致
    }
    return names;
}
