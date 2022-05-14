#pragma once

#ifndef QETRC_MOBILE_2
#include <QWizardPage>
#include "util/buttongroup.hpp"

class QComboBox;
class QSpinBox;

/**
 * @brief The ReadRulerPageConfig class
 * 配置页面  相关的editor直接对Wizard开放
 */
class ReadRulerPageConfig : public QWizardPage
{
    Q_OBJECT;
    RadioButtonGroup<2> *gpMode;
    QComboBox* cbPrec;
    QSpinBox *spStart,*spStop;
    QButtonGroup* gpFilt;
    QSpinBox* spCutSec, *spCutStd;
    QSpinBox* spCutCount;
    friend class ReadRulerWizard;

public:
    ReadRulerPageConfig(QWidget* parent=nullptr);
private :
    void initUI();
private slots:
    void onMeanToggled(bool on);
    void onCutSecToggled(bool on);
    void onCutStdToggled(bool on);
};

#endif
