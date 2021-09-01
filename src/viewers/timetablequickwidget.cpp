#include "timetablequickwidget.h"
#include "model/train/timetablequickmodel.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>

#include <data/common/qesystem.h>
#include "data/train/train.h"

TimetableQuickWidget::TimetableQuickWidget(QWidget *parent):
    QWidget(parent), model(new TimetableQuickModel(this))
{
    initUI();
}

void TimetableQuickWidget::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* ed=new QLineEdit;
    ed->setFocusPolicy(Qt::NoFocus);
    vlay->addWidget(ed);
    edName = ed;
    auto* ck=new QCheckBox(tr("仅显示停车/营业站"));
    ckStopOnly=ck;
    connect(ck,&QCheckBox::toggled,this,&TimetableQuickWidget::onStopOnlyChanged);
    vlay->addWidget(ck);

    table=new QTableView;
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->verticalHeader()->hide();
    table->setModel(model);
    vlay->addWidget(table);
}

void TimetableQuickWidget::onStopOnlyChanged(bool on)
{
    for(int i=0;i<train->stationCount();i++){
        bool hide=( on && model->item(2*i,0)->foreground().color() == Qt::black );
        table->setRowHidden(2*i,hide);
        table->setRowHidden(2*i+1,hide);
    }
}

void TimetableQuickWidget::setTrain(std::shared_ptr<Train> train)
{
    this->train=train;
    refreshData();
}

void TimetableQuickWidget::refreshData()
{
    if(!train)return;
    edName->setText(tr("%1 (%2->%3)").arg(train->trainName().full(),
                                          train->starting().toSingleLiteral(),
                                          train->terminal().toSingleLiteral()));
    edName->setCursorPosition(0);
    model->setTrain(train);
    // 合并单元格
    for(int i=0;i<train->stationCount();i++){
        table->setSpan(2*i,0,2,1);
    }
    table->resizeColumnsToContents();

    onStopOnlyChanged(ckStopOnly->isChecked());
}
