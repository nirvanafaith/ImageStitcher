#pragma once
#include <QString>
#include <QSet>
#include <QStringList>

// XML 图片名校对工具类（纯静态）：
// - 用正则 img\d+\.jpg 全文扫描 xml 提取图片名（与标签名解耦，兼容 <imglink> 和 <图片链接>）
// - 比较文件夹图片名集合与 xml 图片名集合，输出差集
class XmlChecker {
public:
    // 从 xmlPath 全文扫描所有 img\d+\.jpg 图片名，返回小写化的 QSet。
    // 文件不存在或无匹配返回空集合。
    static QSet<QString> extractImageNames(const QString& xmlPath);

    // 比较两个集合，返回差集结构。
    // onlyInFolder = folderNames - xmlNames（图片库有但 xml 没引用）
    // onlyInXml = xmlNames - folderNames（xml 引用但图片库没有）
    // 两个列表均按字符串序升序排序。
    struct DiffResult {
        QStringList onlyInFolder;
        QStringList onlyInXml;
    };
    static DiffResult compare(const QSet<QString>& folderNames,
                              const QSet<QString>& xmlNames);
private:
    XmlChecker() = delete;
};
