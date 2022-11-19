#include "greedypaintpageconstraint.h"
#include "greedypaintpagepaint.h"
#include "greedypaintwizard.h"
#include <dialogs/trainfilter.h>
#include <QMessageBox>

GreedyPaintWizard::GreedyPaintWizard(Diagram& diagram_, QWidget *parent):
    QTabWidget(parent),diagram(diagram_),
    filter(new TrainFilter(diagram_, this)),
    painter(diagram_, filter->getCore())
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

void GreedyPaintWizard::refreshData(std::shared_ptr<Railway> rail)
{
    if (painter.railway() == rail && rail) {
        pgPaint->model()->refreshData();
        pgPaint->setupStationLabels();
    }
}
