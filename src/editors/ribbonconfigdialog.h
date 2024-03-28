#pragma once

#include <QDialog>

class QComboBox;
class QCheckBox;

/**
 * 2024.03.28  The dialog that configs the styles related to the Ribbon.
 * The results are written into SystemJson.
 */
class RibbonConfigDialog : public QDialog
{
    Q_OBJECT;
    QComboBox* cbStyle, *cbTheme;
    QCheckBox* ckCenter;

public:
    RibbonConfigDialog(QWidget* parent=nullptr);

private:
    void initUI();

signals:
    void configApplied(bool themeChanged);

private slots:
    void refreshData();
    void actApply();
    void actOk();
};



