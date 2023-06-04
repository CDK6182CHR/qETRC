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
    QSpinBox* spRowHeight;
    QLineEdit* edDefaultFile;
    QComboBox* cbRibbonStyle;
    QComboBox* cbSysStyle;
    QCheckBox* ckWeaken, * ckTooltip, * ckCentral, * ckStartup, * ckAutoHighlight;
    QCheckBox* ckDrag;
public:
    SystemJsonDialog(QWidget* parent=nullptr);
private:
    void initUI();
private slots:
    void setData();
    void actApply();
};
#endif
