#include "timeinterppagetrain.h"
#include "timeinterpwizard.h"
#include "timeinterppagepreview.h"
#include "util/railrulercombo.h"
#include "data/train/train.h"
#include "data/diagram/diagram.h"
#include "data/diagram/trainadapter.h"
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>

TimeInterpWizard::TimeInterpWizard(Diagram &diagram, QWidget *parent):
    QWizard(parent),diagram(diagram)
{
    setWindowTitle(tr("时刻表插值"));
    resize(700,800);
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void TimeInterpWizard::initUI()
{
    pgTrain=new TimeInterpPageTrain(diagram);
    addPage(pgTrain);
    pgPreview=new TimeInterpPagePreview();
    addPage(pgPreview);
}

void TimeInterpWizard::accept()
{
    QVector<std::shared_ptr<Train>> modified;
    QVector<std::shared_ptr<Train>> data;

    foreach(auto train, trains) {
        auto t = std::make_shared<Train>(*train);   // copy construct
        auto rail = pgTrain->cbRuler->railway();
        auto ruler = pgTrain->cbRuler->ruler();
        t->bindToRailway(rail,diagram.config());
        auto adp = t->adapterFor(*rail);
        if (adp) {
            adp->timetableInterpolation(ruler, pgTrain->ckToStart->isChecked(),
                pgTrain->ckToEnd->isChecked(), pgTrain->cbPrec->currentData().toInt());
            if (t->timetable().size() > train->timetable().size()) {
                // 有数据修改实施
                modified.push_back(train);
                data.push_back(t);
            }
        }
    }

    if (modified.isEmpty()) {
        QMessageBox::information(this, tr("信息"), tr("没有车次受到影响"));
    }
    else {
        emit interpolationApplied(modified, data);
        QMessageBox::information(this, tr("信息"), tr("时刻表插值应用成功，%1个车次"
            "受到影响。").arg(modified.size()));
    }

    QWizard::accept();
}

void TimeInterpWizard::initializePage(int id)
{
    if(id==1){
        trains=pgTrain->selectedTrains();
        pgPreview->setupData(pgTrain->cbRuler->railway(),
                             pgTrain->cbRuler->ruler(),
                             trains);
    }
}
