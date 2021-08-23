#include "forbidwidget.h"

#include <QtWidgets>
#include "model/delegate/qetimedelegate.h"
#include "data/diagram/diagram.h"
#include "util/buttongroup.hpp"

ForbidWidget::ForbidWidget(std::shared_ptr<Forbid> forbid_,bool commitInPlace, QWidget *parent):
    QWidget(parent),forbid(forbid_),model(new ForbidModel(forbid,this)),inplace(commitInPlace)
{
    initUI();
    refreshBasicData();
}

void ForbidWidget::refreshBasicData()
{
    updating=true;
    ckDown->setChecked(forbid->isDownShow());
    ckUp->setChecked(forbid->isUpShow());
    updating=false;
}

void ForbidWidget::refreshData()
{
    refreshBasicData();
    model->refreshData();
}

void ForbidWidget::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;

    auto* hlay=new QHBoxLayout;
    ckDown=new QCheckBox(tr("下行"));
    ckUp=new QCheckBox(tr("上行"));
    hlay->addWidget(ckDown);
    hlay->addWidget(ckUp);
    flay->addRow(tr("显示天窗"),hlay);

    connect(ckDown,&QCheckBox::toggled,this,&ForbidWidget::onDownShowToggled);
    connect(ckUp,&QCheckBox::toggled,this,&ForbidWidget::onUpShowToggled);

    spLength=new QSpinBox;
    spLength->setRange(0,24*60);
    spLength->setSuffix(tr(" 分 (min)"));
    spLength->setSingleStep(10);
    spLength->setValue(120);
    flay->addRow(tr("默认时长"),spLength);
    vlay->addLayout(flay);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->resizeColumnToContents(ForbidModel::ColInterval);
    table->setEditTriggers(QTableView::AllEditTriggers);
    table->setItemDelegateForColumn(ForbidModel::ColStart,
                                    new QETimeDelegate(this,"hh:mm"));
    table->setItemDelegateForColumn(ForbidModel::ColEnd,
                                    new QETimeDelegate(this,"hh:mm"));
    vlay->addWidget(table);
    initContextMenu();

    //其他的列宽还是手动给定..
    table->setColumnWidth(1, 80);
    table->setColumnWidth(2, 80);
    table->setColumnWidth(3, 80);

    auto* g=new ButtonGroup<2>({"确定","还原"});
    g->connectAll(SIGNAL(clicked()),this,{SLOT(onApply()),SLOT(refreshData())});
    vlay->addLayout(g);
}

void ForbidWidget::initContextMenu()
{
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(table, &QWidget::customContextMenuRequested,
        this, &ForbidWidget::onContextMenu);

    context = new QMenu(this);

    using key_t = decltype(Qt::CTRL + Qt::Key_K);
    using slot_t = decltype(SLOT(calculateBegin()));

    std::initializer_list<std::tuple<QString, key_t, QObject*, slot_t>> acts = {
        {tr("复制当前数据到下一行"),(Qt::ALT + Qt::Key_C),this,SLOT(copyToNextRow())},
        {tr("复制当前数据到本方向所有行"),(Qt::ALT + Qt::SHIFT + Qt::Key_C),this,SLOT(copyToAllRows())},
        {tr("计算结束时间"),(Qt::ALT + Qt::Key_E),this,SLOT(calculateEnd())},
        {tr("计算开始时间"),(Qt::ALT + Qt::Key_R),this,SLOT(calculateBegin())},
        {tr("计算所有结束时间"),(Qt::ALT + Qt::SHIFT + Qt::Key_E),this,SLOT(calculateAllEnd())},
        {tr("计算所有开始时间"),(Qt::ALT + Qt::SHIFT + Qt::Key_R),this,SLOT(calculateAllBegin())},
        {tr("复制下行数据到上行"),(Qt::ALT + Qt::Key_U),model,SLOT(copyFromDownToUp())},
        {tr("复制上行数据到下行"),(Qt::ALT + Qt::Key_D),model,SLOT(copyFromUpToDown())}
    };

    enum { IdxText, IdxShortcut, IdxTarget, IdxSlot };

    int row = 0;
    for (auto& p : acts) {
        auto* act = context->addAction(std::get<IdxText>(p));
        act->setShortcut(std::get<IdxShortcut>(p));
        addAction(act);
        connect(act, SIGNAL(triggered()), std::get<IdxTarget>(p), std::get<IdxSlot>(p));
        row++;
        if (row == 2 || row == 6)
            context->addSeparator();
    }
    
}

int ForbidWidget::currentRow()
{
    return table->selectionModel()->currentIndex().row();
}

void ForbidWidget::onApply()
{
    auto nr = model->appliedForbid();
    if (inplace) {
        qDebug() << "ForbidWidget::onApply: INFO: commit in place. " << Qt::endl;
        forbid->swap(*(nr->getForbid(0)));
    }
    else {
        emit forbidChanged(forbid, nr);
    }
}

void ForbidWidget::onDownShowToggled(bool on)
{
    if(on!=forbid->isDownShow()){
        emit forbidShowToggled(forbid, Direction::Down);
    }
}

void ForbidWidget::onUpShowToggled(bool on)
{
    if(on!=forbid->isUpShow()){
        emit forbidShowToggled(forbid, Direction::Up);
    }
}

void ForbidWidget::copyToNextRow()
{
    const auto& idx = table->currentIndex();
    if (idx.isValid() && idx.row() < model->rowCount() - 1) {
        model->copyToNextRow(idx.row());
        table->selectionModel()->setCurrentIndex(idx.siblingAtRow(idx.row() + 1),
            QItemSelectionModel::SelectCurrent);
    }
}

void ForbidWidget::copyToAllRows()
{
    int r = currentRow();
    auto p = forbid->railway().intervalCircByIndex(r);
    if (p) {
        for (p = p->nextInterval(); p; p = p->nextInterval()) {
            copyToNextRow();
        }
    }   
}

void ForbidWidget::calculateBegin()
{
    int r = currentRow();
    if (r >= 0) {
        model->calculateBegin(r, spLength->value());
    }
}

void ForbidWidget::calculateEnd()
{
    int r = currentRow();
    if (r >= 0) {
        model->calculateEnd(r, spLength->value());
    }
}

void ForbidWidget::calculateAllBegin()
{
    auto flag = QMessageBox::question(this, tr("天窗编辑"), tr("在所有区间，依据结束时间和默认"
        "天窗时长，计算开始时间。如开始时间列有未提交的更改将被覆盖。是否确认？"));
    if (flag != QMessageBox::Yes)
        return;
    int r = currentRow();
    if (r >= 0) {
        model->calculateAllBegin(spLength->value());
    }
}

void ForbidWidget::calculateAllEnd()
{
    auto flag = QMessageBox::question(this, tr("天窗编辑"), tr("在所有区间，依据开始时间和默认"
        "天窗时长，计算结束时间。如结束时间列有未提交的更改将被覆盖。是否确认？"));
    if (flag != QMessageBox::Yes)
        return;
    int r = currentRow();
    if (r >= 0) {
        model->calculateAllEnd(spLength->value());
    }
}

void ForbidWidget::onContextMenu(const QPoint& pos)
{
    auto&& p = table->mapToGlobal(pos);
    context->popup(p);
}

ForbidTabWidget::ForbidTabWidget(std::shared_ptr<Railway> railway_, bool commitInPlace,
                                 QWidget *parent):
    QTabWidget(parent), railway(railway_), inplace(commitInPlace)
{
    for(int i=0;i<Forbid::FORBID_COUNT;i++){
        addForbidTab(railway->getForbid(i));
    }
}

void ForbidTabWidget::addForbidTab(std::shared_ptr<Forbid> forbid)
{
    auto* w=new ForbidWidget(forbid, inplace);
    addTab(w, forbid->name());
    connect(w, &ForbidWidget::forbidChanged,
            this, &ForbidTabWidget::forbidChanged);
    connect(w, &ForbidWidget::forbidShowToggled,
            this,&ForbidTabWidget::forbidShowToggled);
}

void ForbidTabWidget::updateAllRailIntervals(std::shared_ptr<Railway> railway, bool equiv)
{
    if (railway != this->railway) {
        return;
    }
    for (int i = 0; i < count(); i++) {
        static_cast<ForbidWidget*>(widget(i))->getModel()->
            updateRailIntervals(railway, equiv);
    }
}

void ForbidTabWidget::refreshData(std::shared_ptr<Forbid> forbid)
{
    static_cast<ForbidWidget*>(widget(forbid->index()))->refreshData();
}

void ForbidTabWidget::refreshBasicData(std::shared_ptr<Forbid> forbid)
{
    static_cast<ForbidWidget*>(widget(forbid->index()))->refreshBasicData();
}
