#pragma once
#include <QImage>
#include <QString>
#include <QSet>
#include <QFuture>
#include <utility>

// 文件服务工具类（纯静态，无需实例化）
class FileManager {
public:
    // 加载单张图片，统一转为 Format_RGB32；失败返回 null QImage
    static QImage loadImage(const QString& folder, const QString& imgName);

    // 保存图片为 JPG，质量 95；path 为完整路径
    static bool saveImage(const QString& path, const QImage& image);

    // 删除文件；返回是否成功（文件不存在视为成功）
    static bool deleteFile(const QString& path);

    // 拼接完整路径：folder + "/" + imgName（自动处理斜杠）
    static QString joinPath(const QString& folder, const QString& imgName);

    // 并行加载两张图片，返回 std::pair<QImage,QImage>（img1 对应 name1，img2 对应 name2）
    // 内部用 QtConcurrent::run 并行加载，再用 .waitForFinished() 同步取结果
    static std::pair<QImage, QImage> loadTwoImagesParallel(const QString& folder,
                                                           const QString& name1,
                                                           const QString& name2);

    // 计算备份目录路径：与原图片文件夹同级，名称 = 原文件夹名 + "_backup"
    // 例：folder="D:/pinjie/imageCut" → 返回 "D:/pinjie/imageCut_backup"
    static QString backupDirFor(const QString& folder);

    // 备份单文件到 backupDir（保留原文件名）。
    // backupDir 不存在则自动创建（QDir::mkpath）。
    // 目标已存在则先 QFile::remove 再 QFile::copy（保证最新原图覆盖旧备份）。
    // srcPath 不存在返回 true（视为无需备份）。
    // 复制失败返回 false。
    static bool backupFile(const QString& srcPath, const QString& backupDir);

    // 便捷批量备份：计算 backupDir=backupDirFor(folder)，逐个 backupFile(joinPath(folder,name), backupDir)
    // 任一失败返回 false
    static bool backupFiles(const QString& folder, const QStringList& fileNames);

    // 扫描文件夹下所有 .jpg/.JPG 文件名，返回小写化的 QSet（供校对用）。
    // 文件夹不存在返回空集合。
    static QSet<QString> listJpgNames(const QString& folder);
private:
    FileManager() = delete;  // 纯静态工具类，禁止实例化
};
