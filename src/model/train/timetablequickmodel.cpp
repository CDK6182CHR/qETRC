#include "timetablequickmodel.h"
#include "data/train/train.h"
#include "util/utilfunc.h"

TimetableQuickModel::TimetableQuickModel(QObject *parent) : QStandardItemModel(parent)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({tr("站名"),tr("时刻"),tr("停时股道")});
}

void TimetableQuickModel::setupModel()
{
    if(!train){
        setRowCount(0);
        return;
    }
    using SI=QStandardItem;
    setRowCount(2*train->timetable().size());
    int row=0;
    for(auto p=train->timetable().begin();p!=train->timetable().end();++p){
        setItem(row,ColName,new SI(p->name.toSingleLiteral()));
        QColor color(Qt::blue);
        if (p->isStopped()){
            //停车的情况
            setTimeItem(row,p->arrive);
            setTimeItem(row+1,p->depart);
            setItem(row,ColNote,new SI(qeutil::secsToString(p->stopSec())));
        }else if(train->isStartingStation(p->name)) {
            //标准始发站情况（停时为0）
            setTimeItem(row+1,p->depart);
            setItem(row,ColNote,new SI(tr("始")));
        }else if(train->isTerminalStation(p->name)){
            //标准终到站
            setTimeItem(row,p->arrive);
            setItem(row+1,ColTime,new SI("--"));
            setItem(row,ColNote,new SI(tr("终")));
        }else{
            //通过站
            color=Qt::black;
            setItem(row,ColTime,new SI("..."));
            setTimeItem(row+1,p->depart);
            takeItem(row,ColNote);
        }
        setItem(row+1,ColNote,new SI(p->track));
        setStationColor(row,color);
        row+=2;
    }
}

void TimetableQuickModel::setTimeItem(int row, const QTime &tm)
{
    setItem(row,ColTime,new QStandardItem(tm.toString("hh:mm:ss")));
}

void TimetableQuickModel::setStationColor(int row, const QColor &color)
{
    // 一共五个Cell，手写
    item(row,ColName)->setForeground(color);
    item(row,ColName)->setTextAlignment(Qt::AlignCenter);

    if(auto* it= item(row,ColTime)){
        it->setForeground(color);
        it->setTextAlignment(Qt::AlignCenter);
    }

    item(row+1,ColTime)->setForeground(color);
    item(row+1,ColTime)->setTextAlignment(Qt::AlignCenter);

    if(auto* it=item(row,ColNote)){
        it->setForeground(color);
    }
    if(auto* it=item(row+1,ColNote)){
        it->setForeground(color);
    }
}

void TimetableQuickModel::refreshData()
{
    setupModel();
}

void TimetableQuickModel::setTrain(std::shared_ptr<Train> train)
{
    this->train=train;
    setupModel();
}

