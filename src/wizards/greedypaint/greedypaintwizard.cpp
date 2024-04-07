#include "greedypaintpageconstraint.h"
#include "greedypaintpagepaint.h"
#include "greedypaintwizard.h"
#include <editors/train/trainfilterselector.h>
#include <data/diagram/diagram.h>
#include <QMessageBox>

GreedyPaintWizard::GreedyPaintWizard(Diagram& diagram_, QWidget *parent):
    QTabWidget(parent),diagram(diagram_),
    filter(new TrainFilterSelector(diagram_.trainCollection())),
    painter(diagram_, filter->core())
{
    setWindowTitle(tr("贪心推线系统"));
    initUI();
}

void GreedyPaintWizard::initUI()
{
    pgConst=new GreedyPaintPageConstraint(diagram,painter,filter,this);
    addTab(pgConst,tr("排图参数"));
    connect(pgConst,&GreedyPaintPageConstraint::actClose,
            this,&GreedyPaintWizard::close);
    connect(pgConst,&GreedyPaintPageConstraint::constraintChanged,
            this,&GreedyPaintWizard::onConstraintChanged);
    connect(pgConst,&GreedyPaintPageConstraint::actClose,
            this,&GreedyPaintWizard::onClose);

    pgPaint=new GreedyPaintPagePaint(diagram,painter,this);
    connect(pgPaint,&GreedyPaintPagePaint::actClose,
            this,&GreedyPaintWizard::onClose);
    connect(pgPaint, &GreedyPaintPagePaint::trainAdded,
        this, &GreedyPaintWizard::trainAdded);
    connect(pgPaint, &GreedyPaintPagePaint::showStatus,
        this, &GreedyPaintWizard::showStatus);
    connect(pgPaint, &GreedyPaintPagePaint::removeTmpTrainLine,
        this, &GreedyPaintWizard::removeTmpTrainLine);
    connect(pgPaint, &GreedyPaintPagePaint::paintTmpTrainLine,
        this, &GreedyPaintWizard::paintTmpTrainLine);
    addTab(pgPaint,tr("铺画"));
}

void GreedyPaintWizard::onConstraintChanged()
{
    setCurrentIndex(1);
    //2022.06.02取消这个不等条件，原因是Ruler也可能有变化，因此每次确认都更新一下
    //if (painter.ruler() != pgPaint->model()->ruler()) {
    pgPaint->model()->setRuler(painter.ruler());
    pgPaint->model()->refreshData();
    pgPaint->setupStationLabels();
    pgPaint->clearInfoWidgets();
    //}
}

void GreedyPaintWizard::onClose()
{
    if(showCloseMsg){
        QMessageBox::information(this,tr("提示"),
                                 tr("关闭本窗口后，下次再进入本功能时，将继续保留当前配置的状态。"
            "但如果手动关闭或者退出程序，信息将会丢失。\n"
            "本提示信息在本窗口存续期间，显示一次。"));
        showCloseMsg=false;
    }
    QTabWidget::close();
}

void GreedyPaintWizard::cleanUpTempData()
{
    pgPaint->clearTmpTrainLine();
}

void GreedyPaintWizard::onPaintingPointClicked(DiagramWidget* d, std::shared_ptr<Train> train, AdapterStation* st)
{
    pgPaint->onPaintingPointClicked(d, train, st);
}

void GreedyPaintWizard::onRulerRemoved(std::shared_ptr<Ruler> ruler, std::shared_ptr<Railway> rail)
{
    if (ruler == painter.ruler()) {
        painter.setRuler(nullptr);
    }
    refreshData(rail);
    setCurrentIndex(0);
}

void GreedyPaintWizard::onRailwayRemoved(std::shared_ptr<Railway> rail)
{
    if (rail == painter.railway()) {
        painter.setRailway(nullptr);
    }
    refreshData(rail);
    setCurrentIndex(0);
}

void GreedyPaintWizard::refreshData(std::shared_ptr<Railway> rail)
{
    pgConst->refreshData();   // 2024.04.07  
    if (painter.railway() == rail && rail) {
        pgPaint->model()->refreshData();
        pgPaint->setupStationLabels();
        pgPaint->clearInfoWidgets();
    }
}
