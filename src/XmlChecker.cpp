#include "XmlChecker.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <algorithm>

QSet<QString> XmlChecker::extractImageNames(const QString& xmlPath) {
    QSet<QString> names;
    QFile file(xmlPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return names;

    QTextStream in(&file);
    in.setCodec("UTF-8");
    const QString content = in.readAll();
    file.close();

    // 正则匹配 img + 数字 + .jpg（不区分大小写，与 NumberUtil 的 img(\d+)\.jpg 一致）
    static const QRegularExpression re(QStringLiteral("img\\d+\\.jpg"),
                                       QRegularExpression::CaseInsensitiveOption);
    auto it = re.globalMatch(content);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        names.insert(m.captured(0).toLower());  // 小写化保证与文件系统一致
    }
    return names;
}

XmlChecker::DiffResult XmlChecker::compare(const QSet<QString>& folderNames,
                                            const QSet<QString>& xmlNames) {
    DiffResult result;
    // 图片库有但 xml 没引用
    for (const QString& name : folderNames) {
        if (!xmlNames.contains(name)) result.onlyInFolder.append(name);
    }
    // xml 引用但图片库没有
    for (const QString& name : xmlNames) {
        if (!folderNames.contains(name)) result.onlyInXml.append(name);
    }
    // 按字符串序升序排序便于展示
    std::sort(result.onlyInFolder.begin(), result.onlyInFolder.end());
    std::sort(result.onlyInXml.begin(), result.onlyInXml.end());
    return result;
}
