#include "greedypaintpageconstraint.h"
#include "greedypaintpagepaint.h"
#include "greedypaintwizard.h"

#include <QMessageBox>

GreedyPaintWizard::GreedyPaintWizard(Diagram& diagram_, QWidget *parent):
    QTabWidget(parent),diagram(diagram_),painter(diagram_)
{
    setWindowTitle(tr("贪心推线系统"));
    initUI();
}

void GreedyPaintWizard::initUI()
{
    pgConst=new GreedyPaintPageConstraint(diagram,painter,this);
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
    addTab(pgPaint,tr("铺画"));
}

void GreedyPaintWizard::onConstraintChanged()
{
    setCurrentIndex(1);
    pgPaint->model()->setRuler(painter.ruler());
    pgPaint->model()->refreshData();
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
