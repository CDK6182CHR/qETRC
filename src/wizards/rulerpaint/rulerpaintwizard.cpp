#include "rulerpaintwizard.h"


#include "data/diagram/diagram.h"

//const  QString
//    RulerPaintWizard::fRail=QStringLiteral("railway"),
//    RulerPaintWizard::fRuler=QStringLiteral("ruler"),
//    RulerPaintWizard::fDir=QStringLiteral("dir"),
//    RulerPaintWizard::fAnchorStation=QStringLiteral("anchorStation");

RulerPaintWizard::RulerPaintWizard(Diagram &diagram_, QWidget *parent):
    QWizard(parent), diagram(diagram_)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("标尺排图向导"));
    resize(800,800);
    initUI();
}

void RulerPaintWizard::initUI()
{
    pgStart=new RulerPaintPageStart(diagram.trainCollection());
    addPage(pgStart);
    pgStation = new RulerPaintPageStation(diagram.railCategory());
    addPage(pgStation);
    pgTable=new RulerPaintPageTable(pgStation);
    addPage(pgTable);
}
