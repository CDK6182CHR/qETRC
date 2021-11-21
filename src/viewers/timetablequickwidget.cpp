#include "timetablequickwidget.h"
#include "model/train/timetablequickmodel.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QMenu>
#include <QScroller>

#include <data/common/qesystem.h>
#include "data/train/train.h"
#include "dialogs/locatedialog.h"

TimetableQuickWidget::TimetableQuickWidget(QUndoStack* undo_, QWidget *parent):
    QWidget(parent),
    model(new TimetableQuickEditableModel(undo_,this))
{
    initUI();
    QScroller::grabGesture(table, QScroller::TouchGesture);
    table->setAttribute(Qt::WA_AcceptTouchEvents);
}

void TimetableQuickWidget::setReadonly()
{
    ckEdit->setChecked(false);
    ckEdit->setEnabled(false);
}

void TimetableQuickWidget::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* ed=new QLineEdit;
    ed->setFocusPolicy(Qt::NoFocus);
    vlay->addWidget(ed);
    edName = ed;
    auto* ck=new QCheckBox(tr("仅停车/营业站"));
    ckStopOnly=ck;
    connect(ck,&QCheckBox::toggled,this,&TimetableQuickWidget::onStopOnlyChanged);
    auto* hlay = new QHBoxLayout;
    hlay->addWidget(ck);
    ck = new QCheckBox(tr("编辑"));
    ck->setToolTip(tr("启用编辑\n如果勾选，则允许直接修改车站时刻表，"
        "修改的时刻立即生效。"));
    hlay->addWidget(ck);
    ckEdit = ck;
    vlay->addLayout(hlay);
    connect(ck, &QCheckBox::toggled, this, &TimetableQuickWidget::onEditCheckChanged);

    table=new QTableView;
    table->setEditTriggers(QTableView::NoEditTriggers);  
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->verticalHeader()->hide();
    table->setModel(model);
    table->setItemDelegateForColumn(TimetableQuickModel::ColTime,
        new TimeQuickDelegate(this));
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(table, &QTableView::customContextMenuRequested,
        this, &TimetableQuickWidget::onTableContext);

    meContext = new QMenu(this);
    auto* act = meContext->addAction(tr("定位到到达点"), this,
        &TimetableQuickWidget::locateArrive, Qt::ALT + Qt::Key_G);
    table->addAction(act);
    act = meContext->addAction(tr("定位到出发点"), this,
        &TimetableQuickWidget::locateDepart, Qt::ALT + Qt::SHIFT + Qt::Key_G);
    table->addAction(act);

    vlay->addWidget(table);
}

void TimetableQuickWidget::onStopOnlyChanged(bool on)
{
    if (!train)
        return;
    for(int i=0;i<train->stationCount();i++){
        bool hide = (on && !model->isAlwaysShow(2 * i));
        table->setRowHidden(2*i,hide);
        table->setRowHidden(2*i+1,hide);
    }
}

void TimetableQuickWidget::setTrain(std::shared_ptr<Train> train)
{
    this->train=train;
    refreshData();
}

void TimetableQuickWidget::onEditCheckChanged(bool on)
{
    if (on) {
        table->setEditTriggers(QTableView::AllEditTriggers);
    }
    else {
        table->setEditTriggers(QTableView::NoEditTriggers);
    }
}

void TimetableQuickWidget::locateArrive()
{
    auto&& idx = table->currentIndex();
    if (!idx.isValid())return;

    auto lst = model->getBoundingList(idx.row());
    if (lst.empty()) {
        QMessageBox::warning(this, tr("错误"),
            tr("指定车站没有铺画点，无法定位到运行图"));
    }
    else {
        emit locateToBoundStation(lst, model->arriveTimeForRow(idx.row()));
    }
}

void TimetableQuickWidget::locateDepart()
{
    auto&& idx = table->currentIndex();
    if (!idx.isValid())return;

    auto lst = model->getBoundingList(idx.row());
    if (lst.empty()) {
        QMessageBox::warning(this, tr("错误"),
            tr("指定车站没有铺画点，无法定位到运行图"));
    }
    else {
        emit locateToBoundStation(lst, model->departTimeForRow(idx.row()));
    }
}

void TimetableQuickWidget::onTableContext(const QPoint& pos)
{
    meContext->popup(table->mapToGlobal(pos));
}

void TimetableQuickWidget::resetTrain()
{
    setTrain(nullptr);
}

void TimetableQuickWidget::refreshData()
{
    model->setTrain(train);
    if(!train)return;
    edName->setText(tr("%1 (%2->%3)").arg(train->trainName().full(),
                                          train->starting().toSingleLiteral(),
                                          train->terminal().toSingleLiteral()));
    edName->setCursorPosition(0);
    // 合并单元格
    for(int i=0;i<train->stationCount();i++){
        table->setSpan(2*i,0,2,1);
    }
    table->resizeColumnsToContents();

    onStopOnlyChanged(ckStopOnly->isChecked());
}

