#include "forbidmodel.h"
#include "util/utilfunc.h"
#include "data/rail/forbid.h"

ForbidModel::ForbidModel(std::shared_ptr<Forbid> forbid_, QObject *parent):
    IntervalDataModel(forbid_->railway(),parent),forbid(forbid_)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
       tr("区间"),tr("起始"),tr("结束"),tr("时长")
                              });
    refreshData();
    connect(this, &QStandardItemModel::dataChanged, this,
        &ForbidModel::onDataChanged);
}

void ForbidModel::refreshData()
{
    setupModel();
}

std::shared_ptr<Railway> ForbidModel::appliedForbid()
{
    auto nr = forbid->clone();
    auto f = nr->getForbid(0);
    int row = 0;
    for (auto p = f->firstDownNode(); p; p = p->nextNodeCirc(),++row) {
        if (!checkRowInterval(p->railInterval(), row))
            continue;
        p->beginTime = item(row, ColStart)->data(Qt::EditRole).toTime();
        p->endTime = item(row, ColEnd)->data(Qt::EditRole).toTime();
    }
    return nr;
}

void ForbidModel::setupModel()
{
    if (!forbid) {
        setRowCount(0);
        return;
    }
    if (!forbid->different()) {
        qDebug() << "ForbidModel::setupModel: WARNING: ruler not `different`: " <<
            forbid->index() << ". It will be set to `different`." << Qt::endl;
        forbid->setDifferent(true);
    }

    IntervalDataModel::setupModel();
}

void ForbidModel::setupRow(int row, std::shared_ptr<RailInterval> railint)
{
    IntervalDataModel::setupRow(row,railint);
    auto node=railint->getForbidNode(forbid);
    using SI=QStandardItem;

    auto* it=new SI;
    auto tm = node->beginTime;
    if (!tm.isValid())tm = QTime(0, 0);
    it->setData(tm,Qt::EditRole);
    setItem(row,ColStart,it);

    it=new SI;
    tm = node->endTime;
    if (!tm.isValid())tm = QTime(0, 0);
    it->setData(tm,Qt::EditRole);
    setItem(row,ColEnd,it);

    it=new SI(qeutil::minsToStringHM(node->durationMin()));
    it->setEditable(false);
    setItem(row,ColLength,it);
}

void ForbidModel::copyRowData(int from, int to)
{
    updating=true;
    setItem(to,ColStart,item(from,ColStart)->clone());
    setItem(to,ColEnd,item(from,ColEnd)->clone());
    setItem(to,ColLength,item(from,ColLength)->clone());
    updating=false;
}

void ForbidModel::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if(std::max((int)ColStart,topLeft.column())<=std::min((int)ColEnd,bottomRight.column())){
        for(int i=topLeft.row();i<=bottomRight.row();i++){
            updateDuration(i);
        }
    }
}

void ForbidModel::updateDuration(int row)
{
    int secs=qeutil::secsTo(item(row,ColStart)->data(Qt::EditRole).toTime(),
                            item(row,ColEnd)->data(Qt::EditRole).toTime());
    auto* it=new QStandardItem(qeutil::minsToStringHM(secs/60));
    it->setEditable(false);
    setItem(row,ColLength,it);
}


void ForbidModel::copyToNextRow(int row)
{
    if (row < rowCount() - 1) {
        item(row + 1, ColStart)->setData(item(row, ColStart)->data(Qt::EditRole), Qt::EditRole);
        item(row + 1, ColEnd)->setData(item(row, ColEnd)->data(Qt::EditRole), Qt::EditRole);
    }
}

void ForbidModel::calculateBegin(int row, int mins)
{
    item(row, ColStart)->setData(
        item(row, ColEnd)->data(Qt::EditRole).toTime().addSecs(-mins * 60), Qt::EditRole);
}

void ForbidModel::calculateEnd(int row, int mins)
{
    item(row, ColEnd)->setData(
        item(row, ColStart)->data(Qt::EditRole).toTime().addSecs(mins * 60), Qt::EditRole);
}

void ForbidModel::calculateAllBegin(int mins)
{
    for (int i = 0; i < rowCount(); i++)
        calculateBegin(i, mins);
}

void ForbidModel::calculateAllEnd(int mins)
{
    for (int i = 0; i < rowCount(); i++)
        calculateEnd(i, mins);
}



