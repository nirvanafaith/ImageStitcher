#pragma once
#include <QMainWindow>
#include <QImage>
#include <QString>
#include "Stitcher.h"  // StitchDirection/AlignMode

// 前置声明（减少编译依赖）
class QLineEdit;
class QPushButton;
class QRadioButton;
class QLabel;
class QButtonGroup;
class QCheckBox;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;  // 窗口缩放时重新缩放预览（不重新拼接）

private slots:
    void onBrowseFolder();
    void onBrowseXml();
    void onPreview();     // 确认预览
    void onOutput();      // 输出
    void onSmartImport();  // 智能导入：选根目录自动找 imageCut 和 xml
    void onCheck();        // 校对图片：比较图片库与 xml 图片名差异
    void onCompleteImg1();  // 补全图片1名称
    void onCompleteImg2();  // 补全图片2名称

private:
    // 左侧输入控件
    QLineEdit *m_folderEdit;
    QLineEdit *m_xmlEdit;
    QLineEdit *m_img1Edit;
    QLineEdit *m_img2Edit;
    QButtonGroup *m_dirGroup;     // 上下/左右
    QButtonGroup *m_alignGroup;   // 居左/中线/居右
    QPushButton *m_previewBtn;
    QPushButton *m_outputBtn;
    QPushButton *m_smartImportBtn;  // 智能导入按钮
    QPushButton *m_checkBtn;        // 校对图片按钮
    QPushButton *m_completeImg1Btn;  // 图片1名称补全按钮
    QPushButton *m_completeImg2Btn;  // 图片2名称补全按钮
    QCheckBox *m_trimBordersCheck;  // 去除白边勾选框
    QCheckBox *m_fillColorCheck;  // 填充颜色勾选框

    // 右侧展示区
    QLabel *m_displayLabel;
    QCheckBox *m_modifyXmlCheck;  // "同时修改 XML" 开关

    // 预览状态缓存
    QImage m_stitchedCache;       // 拼接后原图缓存（resize 时只缩放此缓存，不重新拼接）
    QString m_keepImgName;        // 预览/输出时计算的小序号图名（输出保存用）
    QString m_removeImgName;      // 大序号图名（删除用）

    // 内部辅助
    StitchDirection currentDirection() const;
    AlignMode currentAlign() const;
    void refreshDisplay();        // 按 displayLabel 尺寸缩放 m_stitchedCache 并显示
    bool validateInputs();        // 校验输入，失败弹 QMessageBox::warning 并返回 false
};
