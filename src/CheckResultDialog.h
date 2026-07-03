#pragma once
#include <QDialog>
#include <QStringList>

// 校对结果对话框：双栏 QListWidget 展示「只在 XML 中」和「只在图片库中」的差异
class QListWidget;
class QLabel;

class CheckResultDialog : public QDialog {
    Q_OBJECT
public:
    // onlyInXml: xml 引用但图片库没有的图片名列表
    // onlyInFolder: 图片库有但 xml 没引用的图片名列表
    explicit CheckResultDialog(const QStringList& onlyInXml,
                                const QStringList& onlyInFolder,
                                QWidget *parent = nullptr);
private:
    QListWidget *m_xmlList;
    QListWidget *m_folderList;
    QLabel *m_xmlLabel;
    QLabel *m_folderLabel;
};
