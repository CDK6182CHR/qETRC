#pragma once
#include <QWizard>

#include "rulerpaintpagestart.h"
#include "rulerpaintpagestation.h"
#include "rulerpaintpagetable.h"

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
    RulerPaintPageStation* pgStation;
    RulerPaintPageTable* pgTable;

public:
    /**
     * 存储数据的field名称
     */
//    static const QString fRail,fRuler,fDir,fAnchorStation;

    RulerPaintWizard(Diagram& diagram_, QWidget* parent=nullptr);
private:
    void initUI();
};

