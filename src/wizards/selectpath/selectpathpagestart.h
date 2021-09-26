#pragma once

#include <QWizardPage>

class QSpinBox;
class QCheckBox;
/**
 * 起始页面：目前主要包含标尺选项。
 */
class SelectPathPageStart : public QWizardPage
{
    Q_OBJECT
    QCheckBox* ckRuler;
    QSpinBox* spRulerCount;
public:
    SelectPathPageStart(QWidget* parent=nullptr);

    bool exportRuler()const;
    int minRulerCount()const;
private:
    void initUI();
};

