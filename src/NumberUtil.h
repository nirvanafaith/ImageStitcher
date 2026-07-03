#pragma once
#include <QString>

namespace NumberUtil {
    // 从图片文件名提取数值序号。
    // 文件名形如 img00024002.jpg → 返回 24002
    // 无效文件名返回 -1
    qint64 extractNumber(const QString& imgFileName);

    // 将用户输入补全/规范为 img000xxxxx.jpg 格式。
    // 输入 63003/img63003/img00063003.jpg 均输出 img00063003.jpg；
    // 输入不含数字或为空时返回空字符串。
    QString completeImageName(const QString& input);
}
