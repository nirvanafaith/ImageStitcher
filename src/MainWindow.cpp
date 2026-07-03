#include "MainWindow.h"
#include "FileManager.h"
#include "Stitcher.h"
#include "XmlEditor.h"
#include "NumberUtil.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QResizeEvent>
#include <QPixmap>
#include <QSizePolicy>
#include "PathResolver.h"
#include "XmlChecker.h"
#include "CheckResultDialog.h"
#include <QSet>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("图片拼接工具"));
    resize(1280, 800);

    // 全局极简风格
    setStyleSheet(QStringLiteral(
        "QMainWindow { background-color: #ffffff; }"
        "QLabel { color: #333333; }"
        "QLineEdit { border: 1px solid #cccccc; border-radius: 4px; padding: 4px 6px; background-color: #ffffff; }"
        "QPushButton { background-color: #4a90d9; color: white; border: none; border-radius: 4px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: #5a9fe8; }"
        "QPushButton:pressed { background-color: #3a7fc0; }"
        "QPushButton:disabled { background-color: #b0c4d8; color: #f0f0f0; }"
        "QRadioButton { spacing: 4px; }"
    ));

    // 中央容器
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // ===== 左侧控制面板 =====
    QWidget *panel = new QWidget(central);
    panel->setFixedWidth(360);
    QVBoxLayout *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(0, 0, 0, 0);
    panelLayout->setSpacing(8);

    // 0. 智能导入（一键找 imageCut 和 xml）
    m_smartImportBtn = new QPushButton(QStringLiteral("智能导入根目录"), panel);
    m_smartImportBtn->setToolTip(QStringLiteral("选择根目录后自动向下查找 imageCut 文件夹和第一个 XML 文件，一次性填充两个路径"));
    connect(m_smartImportBtn, &QPushButton::clicked, this, &MainWindow::onSmartImport);
    panelLayout->addWidget(m_smartImportBtn);

    // 1. 图片文件夹
    QLabel *folderLabel = new QLabel(QStringLiteral("图片文件夹："), panel);
    panelLayout->addWidget(folderLabel);
    QHBoxLayout *folderRow = new QHBoxLayout();
    folderRow->setSpacing(6);
    m_folderEdit = new QLineEdit(panel);
    m_folderEdit->setPlaceholderText(QStringLiteral("选择图片所在文件夹"));
    QPushButton *browseFolderBtn = new QPushButton(QStringLiteral("浏览..."), panel);
    connect(browseFolderBtn, &QPushButton::clicked, this, &MainWindow::onBrowseFolder);
    folderRow->addWidget(m_folderEdit);
    folderRow->addWidget(browseFolderBtn);
    panelLayout->addLayout(folderRow);

    // 2. XML 文件
    QLabel *xmlLabel = new QLabel(QStringLiteral("XML 文件："), panel);
    panelLayout->addWidget(xmlLabel);
    QHBoxLayout *xmlRow = new QHBoxLayout();
    xmlRow->setSpacing(6);
    m_xmlEdit = new QLineEdit(panel);
    m_xmlEdit->setPlaceholderText(QStringLiteral("选择源 XML 文件"));
    QPushButton *browseXmlBtn = new QPushButton(QStringLiteral("浏览..."), panel);
    connect(browseXmlBtn, &QPushButton::clicked, this, &MainWindow::onBrowseXml);
    xmlRow->addWidget(m_xmlEdit);
    xmlRow->addWidget(browseXmlBtn);
    panelLayout->addLayout(xmlRow);

    // 3. 图片1名称
    QLabel *img1Label = new QLabel(QStringLiteral("图片1名称(上/左)："), panel);
    panelLayout->addWidget(img1Label);
    QHBoxLayout *img1Row = new QHBoxLayout();
    img1Row->setSpacing(6);
    m_img1Edit = new QLineEdit(panel);
    m_img1Edit->setPlaceholderText(QStringLiteral("img00020002.jpg"));
    m_completeImg1Btn = new QPushButton(QStringLiteral("补全"), panel);
    m_completeImg1Btn->setToolTip(QStringLiteral("将输入补全为 img000xxxxx.jpg 格式"));
    m_completeImg1Btn->setFixedWidth(50);
    connect(m_completeImg1Btn, &QPushButton::clicked, this, &MainWindow::onCompleteImg1);
    img1Row->addWidget(m_img1Edit);
    img1Row->addWidget(m_completeImg1Btn);
    panelLayout->addLayout(img1Row);

    // 4. 图片2名称
    QLabel *img2Label = new QLabel(QStringLiteral("图片2名称(下/右)："), panel);
    panelLayout->addWidget(img2Label);
    QHBoxLayout *img2Row = new QHBoxLayout();
    img2Row->setSpacing(6);
    m_img2Edit = new QLineEdit(panel);
    m_img2Edit->setPlaceholderText(QStringLiteral("img00020003.jpg"));
    m_completeImg2Btn = new QPushButton(QStringLiteral("补全"), panel);
    m_completeImg2Btn->setToolTip(QStringLiteral("将输入补全为 img000xxxxx.jpg 格式"));
    m_completeImg2Btn->setFixedWidth(50);
    connect(m_completeImg2Btn, &QPushButton::clicked, this, &MainWindow::onCompleteImg2);
    img2Row->addWidget(m_img2Edit);
    img2Row->addWidget(m_completeImg2Btn);
    panelLayout->addLayout(img2Row);

    // 5. 去除白边 + 同时修改 XML + 填充颜色（水平并排）
    QHBoxLayout *optionRow = new QHBoxLayout();
    optionRow->setSpacing(12);
    m_trimBordersCheck = new QCheckBox(QStringLiteral("去除白边"), panel);
    m_trimBordersCheck->setChecked(false);  // 默认不勾选，保持原行为
    m_trimBordersCheck->setToolTip(QStringLiteral("勾选后先去除两图四周白色边缘再拼接"));
    m_modifyXmlCheck = new QCheckBox(QStringLiteral("同时修改 XML"), panel);
    m_modifyXmlCheck->setChecked(true);  // 默认勾选，保持原行为
    m_modifyXmlCheck->setToolTip(QStringLiteral(
        "勾选：输出时同步修改源 XML（更新小序号图宽高、删除大序号图段落）\n"
        "不勾选：仅更新图片文件，不修改源 XML"));
    m_fillColorCheck = new QCheckBox(QStringLiteral("填充颜色"), panel);
    m_fillColorCheck->setChecked(false);  // 默认不勾选，保持原行为
    m_fillColorCheck->setToolTip(QStringLiteral(
        "勾选后用两图主色（出现次数最多的像素颜色）填充拼接画布的留白区域\n"
        "不勾选则用白色填充"));
    optionRow->addWidget(m_trimBordersCheck);
    optionRow->addWidget(m_modifyXmlCheck);
    optionRow->addWidget(m_fillColorCheck);
    optionRow->addStretch();
    panelLayout->addLayout(optionRow);

    // 6. 拼接方向
    QLabel *dirLabel = new QLabel(QStringLiteral("拼接方向："), panel);
    panelLayout->addWidget(dirLabel);
    QHBoxLayout *dirRow = new QHBoxLayout();
    dirRow->setSpacing(12);
    m_dirGroup = new QButtonGroup(panel);
    QRadioButton *tbRadio = new QRadioButton(QStringLiteral("上下拼接"), panel);
    QRadioButton *lrRadio = new QRadioButton(QStringLiteral("左右拼接"), panel);
    m_dirGroup->addButton(tbRadio, 0);  // 0 = TopBottom
    m_dirGroup->addButton(lrRadio, 1);  // 1 = LeftRight
    tbRadio->setChecked(true);
    dirRow->addWidget(tbRadio);
    dirRow->addWidget(lrRadio);
    dirRow->addStretch();
    panelLayout->addLayout(dirRow);

    // 7. 对齐方式
    QLabel *alignLabel = new QLabel(QStringLiteral("对齐方式："), panel);
    panelLayout->addWidget(alignLabel);
    QHBoxLayout *alignRow = new QHBoxLayout();
    alignRow->setSpacing(12);
    m_alignGroup = new QButtonGroup(panel);
    QRadioButton *leftRadio = new QRadioButton(QStringLiteral("居左"), panel);
    QRadioButton *centerRadio = new QRadioButton(QStringLiteral("中线"), panel);
    QRadioButton *rightRadio = new QRadioButton(QStringLiteral("居右"), panel);
    QRadioButton *stretchRadio = new QRadioButton(QStringLiteral("拉伸"), panel);
    m_alignGroup->addButton(leftRadio, 0);    // 0 = Left
    m_alignGroup->addButton(centerRadio, 1);  // 1 = Center
    m_alignGroup->addButton(rightRadio, 2);   // 2 = Right
    m_alignGroup->addButton(stretchRadio, 3); // 3 = Stretch
    leftRadio->setChecked(true);
    leftRadio->setToolTip(QStringLiteral("上下拼接：窄图靠左；左右拼接：矮图靠上"));
    centerRadio->setToolTip(QStringLiteral("窄/矮图居中"));
    rightRadio->setToolTip(QStringLiteral("上下拼接：窄图靠右；左右拼接：矮图靠下"));
    stretchRadio->setToolTip(QStringLiteral("上下拼接：窄图等比放大至与宽图同宽；左右拼接：矮图等比放大至与高图同高"));
    alignRow->addWidget(leftRadio);
    alignRow->addWidget(centerRadio);
    alignRow->addWidget(rightRadio);
    alignRow->addWidget(stretchRadio);
    alignRow->addStretch();
    panelLayout->addLayout(alignRow);

    // 8. 确认预览
    m_previewBtn = new QPushButton(QStringLiteral("确认预览"), panel);
    connect(m_previewBtn, &QPushButton::clicked, this, &MainWindow::onPreview);
    panelLayout->addWidget(m_previewBtn);

    // 9. 输出
    m_outputBtn = new QPushButton(QStringLiteral("输出"), panel);
    m_outputBtn->setEnabled(false);
    connect(m_outputBtn, &QPushButton::clicked, this, &MainWindow::onOutput);
    panelLayout->addWidget(m_outputBtn);

    // 10. 校对图片
    m_checkBtn = new QPushButton(QStringLiteral("校对图片"), panel);
    m_checkBtn->setToolTip(QStringLiteral("比较图片库与 XML 中的图片名，列出差异"));
    connect(m_checkBtn, &QPushButton::clicked, this, &MainWindow::onCheck);
    panelLayout->addWidget(m_checkBtn);

    panelLayout->addStretch();
    mainLayout->addWidget(panel);

    // ===== 右侧展示区（上方图片预览 + 下方 XML 修改开关）=====
    QWidget *rightPanel = new QWidget(central);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(8);

    m_displayLabel = new QLabel(rightPanel);
    m_displayLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_displayLabel->setAlignment(Qt::AlignCenter);
    m_displayLabel->setMinimumSize(400, 400);
    m_displayLabel->setStyleSheet(QStringLiteral("background-color: #f0f0f0;"));
    m_displayLabel->setText(QStringLiteral("预览区域"));
    rightLayout->addWidget(m_displayLabel, 1);

    mainLayout->addWidget(rightPanel, 1);
}

void MainWindow::onBrowseFolder()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this, QStringLiteral("选择图片文件夹"));
    if (!dir.isEmpty()) {
        m_folderEdit->setText(dir);
    }
}

void MainWindow::onBrowseXml()
{
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择 XML 文件"), QString(),
        QStringLiteral("XML 文件 (*.xml)"));
    if (!path.isEmpty()) {
        m_xmlEdit->setText(path);
    }
}

StitchDirection MainWindow::currentDirection() const
{
    return (m_dirGroup->checkedId() == 1) ? StitchDirection::LeftRight
                                          : StitchDirection::TopBottom;
}

AlignMode MainWindow::currentAlign() const
{
    switch (m_alignGroup->checkedId()) {
    case 1:  return AlignMode::Center;
    case 2:  return AlignMode::Right;
    case 3:  return AlignMode::Stretch;
    default: return AlignMode::Left;
    }
}

bool MainWindow::validateInputs()
{
    const QString folder = m_folderEdit->text().trimmed();
    if (folder.isEmpty() || !QDir(folder).exists()) {
        QMessageBox::warning(this, QStringLiteral("输入错误"),
                             QStringLiteral("图片文件夹路径无效"));
        return false;
    }

    const QString xmlPath = m_xmlEdit->text().trimmed();
    if (xmlPath.isEmpty() || !QFile::exists(xmlPath)) {
        QMessageBox::warning(this, QStringLiteral("输入错误"),
                             QStringLiteral("XML 文件路径无效"));
        return false;
    }

    const QString name1 = m_img1Edit->text().trimmed();
    const QString name2 = m_img2Edit->text().trimmed();
    if (name1.isEmpty() || name2.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("输入错误"),
                             QStringLiteral("请填写两个图片名称"));
        return false;
    }

    const QString path1 = FileManager::joinPath(folder, name1);
    if (!QFile::exists(path1)) {
        QMessageBox::warning(this, QStringLiteral("输入错误"),
                             QStringLiteral("图片1不存在：%1").arg(name1));
        return false;
    }

    const QString path2 = FileManager::joinPath(folder, name2);
    if (!QFile::exists(path2)) {
        QMessageBox::warning(this, QStringLiteral("输入错误"),
                             QStringLiteral("图片2不存在：%1").arg(name2));
        return false;
    }

    const qint64 n1 = NumberUtil::extractNumber(name1);
    const qint64 n2 = NumberUtil::extractNumber(name2);
    if (n1 == n2) {
        QMessageBox::warning(this, QStringLiteral("输入错误"),
                             QStringLiteral("两张图片序号相同，无法拼接"));
        return false;
    }

    return true;
}

void MainWindow::onPreview()
{
    if (!validateInputs()) return;

    const QString folder = m_folderEdit->text().trimmed();
    const QString name1 = m_img1Edit->text().trimmed();
    const QString name2 = m_img2Edit->text().trimmed();
    const QString xmlPath = m_xmlEdit->text().trimmed();

    // pageCode 一致性校验
    const QString code1 = XmlEditor::readPageCode(xmlPath, name1);
    const QString code2 = XmlEditor::readPageCode(xmlPath, name2);
    if (!code1.isEmpty() && !code2.isEmpty() && code1 != code2) {
        const auto ret = QMessageBox::question(
            this, QStringLiteral("提示"),
            QStringLiteral("两图 pageCode 不一致（%1 与 %2），是否继续？")
                .arg(code1, code2),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret != QMessageBox::Yes) return;
    }

    // 并行加载两图
    const auto pair = FileManager::loadTwoImagesParallel(folder, name1, name2);
    const QImage &img1 = pair.first;
    const QImage &img2 = pair.second;
    if (img1.isNull() || img2.isNull()) {
        QMessageBox::warning(this, QStringLiteral("加载失败"),
                             QStringLiteral("无法加载图片"));
        return;
    }

    const StitchDirection dir = currentDirection();
    const AlignMode align = currentAlign();

    m_stitchedCache = Stitcher::stitch(img1, img2, dir, align,
                                       m_trimBordersCheck->isChecked(),
                                       m_fillColorCheck->isChecked());
    if (m_stitchedCache.isNull()) {
        QMessageBox::warning(this, QStringLiteral("拼接失败"),
                             QStringLiteral("拼接失败"));
        return;
    }

    // 计算小/大序号
    const qint64 n1 = NumberUtil::extractNumber(name1);
    const qint64 n2 = NumberUtil::extractNumber(name2);
    if (n1 <= n2) {
        m_keepImgName = name1;
        m_removeImgName = name2;
    } else {
        m_keepImgName = name2;
        m_removeImgName = name1;
    }

    refreshDisplay();
    m_outputBtn->setEnabled(true);
}

void MainWindow::refreshDisplay()
{
    if (m_stitchedCache.isNull()) {
        m_displayLabel->setText(QStringLiteral("预览区域"));
        return;
    }
    const QSize area = m_displayLabel->size();
    const QPixmap pm = QPixmap::fromImage(m_stitchedCache)
                           .scaled(area, Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
    m_displayLabel->setPixmap(pm);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    refreshDisplay();  // 仅缩放缓存，不重新拼接
}

void MainWindow::onOutput()
{
    if (m_stitchedCache.isNull() || m_keepImgName.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"),
                             QStringLiteral("请先预览"));
        return;
    }

    const bool modifyXml = m_modifyXmlCheck->isChecked();
    const QString folder = m_folderEdit->text().trimmed();
    const QString xmlPath = m_xmlEdit->text().trimmed();
    const QString keepPath = FileManager::joinPath(folder, m_keepImgName);
    const QString removePath = FileManager::joinPath(folder, m_removeImgName);
    const QString backupDir = FileManager::backupDirFor(folder);
    const int newW = m_stitchedCache.width();
    const int newH = m_stitchedCache.height();

    // 动态构建确认对话框文案
    QString msg = QStringLiteral("将执行以下操作：\n"
                                 "1. 备份原图至 %1\n"
                                 "2. 用拼接图覆盖 %2\n"
                                 "3. 删除 %3\n").arg(backupDir, m_keepImgName, m_removeImgName);
    if (modifyXml) {
        msg += QStringLiteral("4. 修改源 XML（更新宽高 + 删除大序号段落）\n");
    } else {
        msg += QStringLiteral("（不修改源 XML）\n");
    }
    msg += QStringLiteral("\n确认继续？");

    const auto ret = QMessageBox::question(
        this, QStringLiteral("确认输出"), msg,
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    // 1. 备份原图（先于覆盖/删除，保护原图）
    if (!FileManager::backupFile(keepPath, backupDir)) {
        QMessageBox::warning(this, QStringLiteral("错误"),
                             QStringLiteral("备份 %1 失败，已终止输出（原图未被修改）").arg(m_keepImgName));
        return;
    }
    if (!FileManager::backupFile(removePath, backupDir)) {
        QMessageBox::warning(this, QStringLiteral("错误"),
                             QStringLiteral("备份 %1 失败，已终止输出（原图未被修改）").arg(m_removeImgName));
        return;
    }

    // 2. 覆盖小序号图
    if (!FileManager::saveImage(keepPath, m_stitchedCache)) {
        QMessageBox::warning(this, QStringLiteral("错误"),
                             QStringLiteral("保存拼接图失败（原图已备份）"));
        return;
    }
    // 3. 删除大序号图
    if (!FileManager::deleteFile(removePath)) {
        QMessageBox::warning(this, QStringLiteral("错误"),
                             QStringLiteral("删除大序号图片失败（原图已备份）"));
        return;
    }
    // 4. 条件修改 XML
    if (modifyXml) {
        if (!XmlEditor::updateAndRemove(xmlPath, m_keepImgName, newW, newH, m_removeImgName)) {
            QMessageBox::warning(this, QStringLiteral("错误"),
                                 QStringLiteral("修改 XML 失败（图片已更新，原图已备份）"));
            return;
        }
    }

    // 成功提示
    QString doneMsg = QStringLiteral("输出完成：已备份原图、覆盖小序号图、删除大序号图");
    if (modifyXml) doneMsg += QStringLiteral("、修改源 XML");
    QMessageBox::information(this, QStringLiteral("完成"), doneMsg);

    m_outputBtn->setEnabled(false);   // 需重新预览才能再输出
    m_stitchedCache = QImage();       // 清空缓存
    m_displayLabel->setText(QStringLiteral("预览区域"));
    m_displayLabel->setPixmap(QPixmap());
}

void MainWindow::onSmartImport()
{
    const QString root = QFileDialog::getExistingDirectory(
        this, QStringLiteral("选择根目录（软件将向下查找 imageCut 文件夹和第一个 XML）"));
    if (root.isEmpty()) return;

    QString imageCutDir, xmlPath;
    if (!PathResolver::resolveImageCutAndXml(root, imageCutDir, xmlPath)) {
        QMessageBox::warning(this, QStringLiteral("未找到"),
                             QStringLiteral("在所选目录下未同时找到 imageCut 文件夹和 XML 文件。\n"
                                            "请确认目录结构，或使用下方“浏览...”按钮手动选择。"));
        return;
    }
    m_folderEdit->setText(imageCutDir);
    m_xmlEdit->setText(xmlPath);
}

void MainWindow::onCheck()
{
    const QString folder = m_folderEdit->text().trimmed();
    const QString xmlPath = m_xmlEdit->text().trimmed();
    if (folder.isEmpty() || !QDir(folder).exists()) {
        QMessageBox::warning(this, QStringLiteral("输入错误"),
                             QStringLiteral("图片文件夹路径无效，请先导入或选择"));
        return;
    }
    if (xmlPath.isEmpty() || !QFile::exists(xmlPath)) {
        QMessageBox::warning(this, QStringLiteral("输入错误"),
                             QStringLiteral("XML 文件路径无效，请先导入或选择"));
        return;
    }

    const QSet<QString> folderNames = FileManager::listJpgNames(folder);
    const QSet<QString> xmlNames = XmlChecker::extractImageNames(xmlPath);
    const XmlChecker::DiffResult diff = XmlChecker::compare(folderNames, xmlNames);

    CheckResultDialog dlg(diff.onlyInXml, diff.onlyInFolder, this);
    dlg.exec();
}

void MainWindow::onCompleteImg1()
{
    const QString text = NumberUtil::completeImageName(m_img1Edit->text());
    if (!text.isEmpty()) {
        m_img1Edit->setText(text);
    }
}

void MainWindow::onCompleteImg2()
{
    const QString text = NumberUtil::completeImageName(m_img2Edit->text());
    if (!text.isEmpty()) {
        m_img2Edit->setText(text);
    }
}
