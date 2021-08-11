#pragma once
#include <QWizard>

#include "rulerpaintpagestart.h"

class Diagram;

/**
 * @brief The RulerPaintWizard class
 * pyETRC.rulerPainter
 * 标尺排图向导，改用QWizard实现
 */
class RulerPaintWizard : public QWizard
{
    Q_OBJECT;
    Diagram& diagram;
    RulerPaintPageStart* pgStart;
public:
    RulerPaintWizard(Diagram& diagram_, QWidget* parent=nullptr);
private:
    void initUI();
};

