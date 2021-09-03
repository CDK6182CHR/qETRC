#include "readrulerpagetrain.h"
#include "data/diagram/diagram.h"
#include "util/buttongroup.hpp"
#include "dialogs/trainfilter.h"
#include "data/diagram/diagram.h"
#include <QtWidgets>
#include <algorithm>

ReadRulerPageTrain::ReadRulerPageTrain(Diagram& diagram_, QWidget *parent):
    QWizardPage(parent),diagram(diagram_),  coll(diagram_.trainCollection())
{
    filter = new TrainFilter(diagram, this);
    connect(filter, &TrainFilter::filterApplied,
        this, &ReadRulerPageTrain::filtTrainApplied);
    initUI();
}

bool ReadRulerPageTrain::validatePage()
{
    if(trains().size()<2){
        QMessageBox::warning(this,tr("错误"),tr("请至少选择两个车次。\n"
        "如需从单个车次读取，可以使用单车次标尺提取功能。"));
        return false;
    }
    return true;
}

void ReadRulerPageTrain::initUI()
{
    setTitle(tr("选择车次"));
    setSubTitle(tr("请选择一组用于读取标尺的车次（通过添加到右边表格）\n"
                       "本系统将认为所选车次属于同一标尺。如果不是，计算准确性受到影响。"));
    auto* vlay=new QVBoxLayout(this);
    auto* g=new ButtonGroup<3>({"车次筛选器","全选显示车次","清空选择"});
    g->connectAll(SIGNAL(clicked()),this,{SLOT(filtTrain()),SLOT(selectAll()),
                                          SLOT(deselectAll())});
    vlay->addLayout(g);
    auto* hlay=new QHBoxLayout;
    tbUnsel=new QTableView;
    mdUnsel=new TrainListReadModel(coll.trains(), this);
    tbUnsel->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    tbUnsel->setModel(mdUnsel);
    hlay->addWidget(tbUnsel);
    tbUnsel->resizeColumnsToContents();
    tbUnsel->setSelectionBehavior(QTableView::SelectRows);
    tbUnsel->setSelectionMode(QTableView::MultiSelection);
    connect(tbUnsel->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            tbUnsel,SLOT(sortByColumn(int,Qt::SortOrder)));
    tbUnsel->horizontalHeader()->setSortIndicatorShown(true);

    auto* cv=new ButtonGroup<2,QVBoxLayout>({">","<"});
    hlay->addLayout(cv);
    cv->setFixedWidth(30);
    cv->connectAll(SIGNAL(clicked()),this,{SLOT(select()),SLOT(deselect())});

    tbSel=new QTableView;
    mdSel=new TrainListReadModel(this);
    tbSel->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    tbSel->setModel(mdSel);
    tbSel->setSelectionBehavior(QTableView::SelectRows);
    tbSel->setSelectionMode(QTableView::MultiSelection);
    connect(tbSel->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            tbSel,SLOT(sortByColumn(int,Qt::SortOrder)));
    tbSel->horizontalHeader()->setSortIndicatorShown(true);
    hlay->addWidget(tbSel);
    vlay->addLayout(hlay);
}

void ReadRulerPageTrain::resizeTables()
{
    tbSel->resizeColumnsToContents();
    tbUnsel->resizeColumnsToContents();
}

QVector<int> ReadRulerPageTrain::inversedSelectedRows(QTableView *table)
{
    auto sel=table->selectionModel()->selectedRows();
    QVector<int> res;
    res.reserve(sel.size());
    foreach(const auto& p, sel){
        res.append(p.row());
    }
    std::sort(res.begin(),res.end(),std::greater<int>());
    return res;
}


void ReadRulerPageTrain::filtTrain()
{
    filter->show();
}

void ReadRulerPageTrain::filtTrainApplied()
{
    for (int i = 0; i < mdUnsel->rowCount({}); i++) {
        auto train = mdUnsel->trains().at(i);
        tbUnsel->setRowHidden(i, !filter->check(train));
    }
}

void ReadRulerPageTrain::selectAll()
{  
    // 2021.09.03 引入筛选器机制，原来的直接take不能用了
    //mdSel->appendTrains(mdUnsel->trains());
    //mdUnsel->clearTrains();
    //tbSel->resizeColumnsToContents();
    QList<std::shared_ptr<Train>> lst;
    for (int i = mdUnsel->rowCount({}) - 1; i >= 0; i--) {
        if (!tbUnsel->isRowHidden(i)) {
            lst.prepend(mdUnsel->takeTrainAt(i));
        }
    }
    mdSel->appendTrains(lst);
    tbSel->resizeColumnsToContents();
}

void ReadRulerPageTrain::deselectAll()
{
    mdUnsel->appendTrains(mdSel->trains());
    mdSel->clearTrains();
    tbUnsel->resizeColumnsToContents();
}

void ReadRulerPageTrain::select()
{
    auto rows=inversedSelectedRows(tbUnsel);
    QList<std::shared_ptr<Train>> lst;
    foreach(int r,rows){
        auto t=mdUnsel->takeTrainAt(r);
        lst.prepend(t);
    }
    mdSel->appendTrains(lst);
    resizeTables();
}

void ReadRulerPageTrain::deselect()
{
    auto rows=inversedSelectedRows(tbSel);
    QList<std::shared_ptr<Train>> lst;
    foreach(int r,rows){
        auto t=mdSel->takeTrainAt(r);
        lst.prepend(t);
    }
    mdUnsel->appendTrains(lst);
    resizeTables();
}
