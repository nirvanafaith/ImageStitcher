#include "XmlEditor.h"
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>
#include <QDomNode>
#include <QTextStream>
#include <QFileInfo>

// 从 <图片链接> 文本（如 "imageCut\img00024002.jpg"）提取纯文件名
static QString extractFileName(const QString& linkText) {
    // 兼容反斜杠与正斜杠
    QString normalized = linkText;
    normalized.replace('\\', '/');
    int idx = normalized.lastIndexOf('/');
    return idx >= 0 ? normalized.mid(idx + 1) : normalized;
}

bool XmlEditor::updateAndRemove(const QString& xmlPath,
                                const QString& keepImgName, int newW, int newH,
                                const QString& removeImgName)
{
    QFile file(xmlPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QDomDocument doc;
    QString errMsg; int errLine = 0, errCol = 0;
    if (!doc.setContent(&file, &errMsg, &errLine, &errCol)) {
        file.close();
        return false;
    }
    file.close();

    QDomNodeList insertions = doc.elementsByTagName(QStringLiteral("插图"));
    bool keepUpdated = false;
    QDomNode removeParagraph;  // 待删除的 <段落> 节点

    for (int i = 0; i < insertions.size(); ++i) {
        QDomNode insertionNode = insertions.at(i);
        // 在当前 <插图> 下找 <图片链接>
        QDomElement linkElem = insertionNode.firstChildElement(QStringLiteral("图片链接"));
        if (linkElem.isNull()) continue;
        QString linkFile = extractFileName(linkElem.text().trimmed());

        if (linkFile == keepImgName) {
            // 更新 <图片宽度> 与 <图片高度>
            QDomElement wElem = insertionNode.firstChildElement(QStringLiteral("图片宽度"));
            QDomElement hElem = insertionNode.firstChildElement(QStringLiteral("图片高度"));
            if (!wElem.isNull()) wElem.firstChild().setNodeValue(QString::number(newW));
            if (!hElem.isNull()) hElem.firstChild().setNodeValue(QString::number(newH));
            keepUpdated = true;
        } else if (linkFile == removeImgName) {
            // 找到所在 <段落>（<插图> 的父节点即 <段落>）
            QDomNode paragraph = insertionNode.parentNode();
            if (!paragraph.isNull()) {
                removeParagraph = paragraph;
            }
        }
    }

    if (!keepUpdated) return false;
    if (removeParagraph.isNull()) {
        // 找不到待删除项也算失败
        return false;
    }
    // 从 <段落> 的父节点中移除该 <段落>
    QDomNode parent = removeParagraph.parentNode();
    if (parent.isNull()) return false;
    parent.removeChild(removeParagraph);

    // 覆盖写回原文件
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return false;
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(false);  // 不写 BOM，保持与原文件一致
    out << doc.toString(4);  // 4 空格缩进
    out.flush();
    file.close();
    return true;
}

QString XmlEditor::readPageCode(const QString& xmlPath, const QString& imgName)
{
    QFile file(xmlPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();

    QDomDocument doc;
    if (!doc.setContent(&file)) { file.close(); return QString(); }
    file.close();

    QDomNodeList insertions = doc.elementsByTagName(QStringLiteral("插图"));
    for (int i = 0; i < insertions.size(); ++i) {
        QDomNode node = insertions.at(i);
        QDomElement linkElem = node.firstChildElement(QStringLiteral("图片链接"));
        if (linkElem.isNull()) continue;
        if (extractFileName(linkElem.text().trimmed()) == imgName) {
            QDomElement insertionElem = node.toElement();
            return insertionElem.attribute(QStringLiteral("pageCode"));
        }
    }
    return QString();
}
