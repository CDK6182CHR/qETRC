#pragma once
#include <QWizard>

#include "readrulerpageinterval.h"

class Diagram;

/**
 * @brief The ReadRulerWizard class
 * 标尺综合 （Ctrl+Shift+B）
 */
class ReadRulerWizard : public QWizard
{
    Q_OBJECT;
    Diagram& diagram;
    ReadRulerPageInterval* pgInterval;
public:
    ReadRulerWizard(Diagram& diagram_, QWidget* parent=nullptr);
private :
    void initUI();
    void initStartPage();
};

