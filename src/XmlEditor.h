#pragma once
#include <QString>

class XmlEditor {
public:
    // 在源 XML 上原地修改：
    // - 更新 keepImgName 所在 <插图> 的 <图片宽度> 为 newW、<图片高度> 为 newH
    // - 删除 removeImgName 所在的整个 <段落> 块
    // - 覆盖写回 xmlPath，不产生新 XML 文件
    // keepImgName / removeImgName 仅传文件名（如 "img00024002.jpg"），匹配时只比较 <图片链接> 文本中的文件名部分
    // 成功返回 true，失败返回 false
    static bool updateAndRemove(const QString& xmlPath,
                                const QString& keepImgName, int newW, int newH,
                                const QString& removeImgName);

    // 读取某张图片所在 <插图> 的 pageCode 属性；找不到返回空字符串
    // imgName 仅传文件名
    static QString readPageCode(const QString& xmlPath, const QString& imgName);
};
