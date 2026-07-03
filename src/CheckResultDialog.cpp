#include "CheckResultDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

CheckResultDialog::CheckResultDialog(const QStringList& onlyInXml,
                                     const QStringList& onlyInFolder,
                                     QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("校对结果"));
    resize(700, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // 双栏区域
    QHBoxLayout *columnsLayout = new QHBoxLayout();
    columnsLayout->setSpacing(12);

    // 左栏：只在 XML 中
    QVBoxLayout *xmlColumn = new QVBoxLayout();
    xmlColumn->setSpacing(4);
    m_xmlLabel = new QLabel(QStringLiteral("只在 XML 中（%1 条）").arg(onlyInXml.size()), this);
    m_xmlList = new QListWidget(this);
    m_xmlList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_xmlList->setSortingEnabled(true);
    m_xmlList->setMinimumWidth(300);
    m_xmlList->setMinimumHeight(360);
    if (onlyInXml.isEmpty()) {
        m_xmlList->addItem(QStringLiteral("（无差异）"));
    } else {
        m_xmlList->addItems(onlyInXml);
    }
    xmlColumn->addWidget(m_xmlLabel);
    xmlColumn->addWidget(m_xmlList, 1);
    columnsLayout->addLayout(xmlColumn);

    // 右栏：只在图片库中
    QVBoxLayout *folderColumn = new QVBoxLayout();
    folderColumn->setSpacing(4);
    m_folderLabel = new QLabel(QStringLiteral("只在图片库中（%1 条）").arg(onlyInFolder.size()), this);
    m_folderList = new QListWidget(this);
    m_folderList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_folderList->setSortingEnabled(true);
    m_folderList->setMinimumWidth(300);
    m_folderList->setMinimumHeight(360);
    if (onlyInFolder.isEmpty()) {
        m_folderList->addItem(QStringLiteral("（无差异）"));
    } else {
        m_folderList->addItems(onlyInFolder);
    }
    folderColumn->addWidget(m_folderLabel);
    folderColumn->addWidget(m_folderList, 1);
    columnsLayout->addLayout(folderColumn);

    mainLayout->addLayout(columnsLayout, 1);

    // 底部关闭按钮
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    QPushButton *closeBtn = new QPushButton(QStringLiteral("关闭"), this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(closeBtn);
    mainLayout->addLayout(btnRow);

    // 极简风格（与主窗口一致）
    setStyleSheet(QStringLiteral(
        "QDialog { background-color: #ffffff; }"
        "QLabel { color: #333333; }"
        "QListWidget { border: 1px solid #cccccc; border-radius: 4px; background-color: #ffffff; }"
        "QPushButton { background-color: #4a90d9; color: white; border: none; border-radius: 4px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: #5a9fe8; }"
        "QPushButton:pressed { background-color: #3a7fc0; }"
    ));
}
