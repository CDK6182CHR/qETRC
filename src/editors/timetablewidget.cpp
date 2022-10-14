#include "timetablewidget.h"

#include <model/train/timetablestdmodel.h>
#include <QAction>
#include <QTableView>
#include <QHeaderView>
#include <data/common/qesystem.h>
#include <model/delegate/qetimedelegate.h>

TimetableWidget::TimetableWidget(bool commitInPlace, QWidget *parent):
    QEControlledTable(parent), _model(new TimetableStdModel(commitInPlace, this))
{
    initUI();
}

void TimetableWidget::refreshData()
{
    _model->refreshData();
}

void TimetableWidget::initUI()
{
    table()->setModel(_model);
    table()->setEditTriggers(QTableView::AllEditTriggers);
    table()->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table()->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    auto* dele=new QETimeDelegate(this);
    table()->setItemDelegateForColumn(TimetableStdModel::ColArrive,dele);
    table()->setItemDelegateForColumn(TimetableStdModel::ColDepart,dele);

    int c = 0;
    for (int w : {120, 100, 100, 40, 60, 60, 60}) {
        table()->setColumnWidth(c++, w);
    }

    auto* act=new QAction(tr("复制到达时刻为出发时刻"), this);
    act->setShortcut(Qt::ALT+Qt::Key_D);
    connect(act,&QAction::triggered,this,&TimetableWidget::copyToDepart);
    addAction(act);

    act=new QAction(tr("复制出发时刻为到达时刻"), this);
    act->setShortcut(Qt::ALT+Qt::SHIFT+Qt::Key_D);
    connect(act,&QAction::triggered,this,&TimetableWidget::copyToArrive);
    addAction(act);

    setContextMenuPolicy(Qt::ActionsContextMenu);
}

void TimetableWidget::copyToDepart()
{
    _model->copyToDepart(table()->currentIndex().row());
}

void TimetableWidget::copyToArrive()
{
    _model->copyToArrive(table()->currentIndex().row());
}
