#include "rulerpaintwizard.h"

#include "data/diagram/diagram.h"

RulerPaintWizard::RulerPaintWizard(Diagram &diagram_, QWidget *parent):
    QWizard(parent), diagram(diagram_)
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(800,800);
    initUI();
}

void RulerPaintWizard::initUI()
{
    pgStart=new RulerPaintPageStart(diagram.trainCollection());
    addPage(pgStart);
}
