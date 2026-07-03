#include "NumberUtil.h"
#include <QRegularExpression>
#include <QFileInfo>

namespace NumberUtil {
    qint64 extractNumber(const QString& imgFileName) {
        // 仅取文件名部分比较，避免路径干扰
        const QString name = QFileInfo(imgFileName).fileName();

        // 大小写不敏感匹配 img<数字>.jpg
        static const QRegularExpression re(QStringLiteral("img(\\d+)\\.jpg"),
                                           QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch m = re.match(name);
        if (!m.hasMatch()) {
            return -1;
        }

        bool ok = false;
        const qint64 num = m.captured(1).toLongLong(&ok);
        return ok ? num : -1;
    }

    QString completeImageName(const QString& input) {
        const QString s = input.trimmed();
        if (s.isEmpty()) return QString();

        // 已是标准格式 img000xxxxx.jpg，直接小写化返回
        static const QRegularExpression standardRe(
            QStringLiteral("^img\\d{8}\\.jpg$"),
            QRegularExpression::CaseInsensitiveOption);
        if (standardRe.match(s).hasMatch()) return s.toLower();

        // 提取连续数字
        static const QRegularExpression digitRe(QStringLiteral("(\\d+)"));
        const auto m = digitRe.match(s);
        if (!m.hasMatch()) return QString();

        bool ok = false;
        const quint64 num = m.captured(1).toULongLong(&ok);
        if (!ok) return QString();

        // 格式化为 img + 至少8位数字（前导零）+ .jpg
        return QStringLiteral("img%1.jpg").arg(num, 8, 10, QLatin1Char('0'));
    }
}
