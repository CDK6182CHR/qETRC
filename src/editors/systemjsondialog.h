#pragma once

#ifndef QETRC_MOBILE_2
#include <QDialog>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;

/**
 * @brief The SystemJsonDialog class
 * 全局系统设置  对SystemJson设置项的UI展示
 */
class SystemJsonDialog : public QDialog
{
    Q_OBJECT
    QComboBox* cbLanguage;
    QSpinBox* spRowHeight;
    QLineEdit* edDefaultFile;
    //QComboBox* cbRibbonStyle;  // 2024.03.28: move to another dialog
    QComboBox* cbSysStyle;
    QCheckBox* ckWeaken, * ckTooltip, * ckCentral, * ckStartup, * ckAutoHighlight;
    QCheckBox* ckDrag, * ckTransparentConfig;
public:
    SystemJsonDialog(QWidget* parent=nullptr);
private:
    void initUI();

    void setLanguageCombo();
private slots:
    void setData();
    void actApply();
};
#endif
