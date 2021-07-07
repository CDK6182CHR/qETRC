#include "timetablestdmodel.h"



TimetableStdModel::TimetableStdModel(bool inplace, QObject *parent):
    QEMoveableModel(parent), commitInPlace(inplace)
{

}

void TimetableStdModel::setTrain(std::shared_ptr<Train> train)
{
    _train = train;
    setupModel();
}

void TimetableStdModel::refreshData()
{
    setupModel();
}

void TimetableStdModel::setupNewRow(int row)
{
    using SI=QStandardItem;
    setItem(row,ColName,new SI);
    auto* it=new SI;
    it->setData(QTime(),Qt::EditRole);
    setItem(row,ColArrive,it);

    it=new SI;
    it->setData(QTime(),Qt::EditRole);
    setItem(row,ColDepart,it);

    setItem(row,ColBusiness,makeCheckItem());
    setItem(row,ColTrack,new SI);
    setItem(row,ColNote,new SI);
}

void TimetableStdModel::setupModel()
{
    if(!_train){
        setRowCount(0);
        return;
    }
    using SI=QStandardItem;
    setRowCount(_train->timetable().size());
    setColumnCount(ColMAX);
    int row=0;

    //注意Timetable是std::list，不能用随机访问
    for(auto p=_train->timetable().begin();p!=_train->timetable().end();
        ++p,++row)
    {
        setItem(row,ColName,new SI(p->name.toSingleLiteral()));

        auto* it=new SI;
        it->setData(p->arrive,Qt::EditRole);
        setItem(row,ColArrive,it);

        it=new SI;
        it->setData(p->depart,Qt::EditRole);
        setItem(row,ColDepart,it);

        it=makeCheckItem();
        it->setCheckState(qeutil::boolToCheckState(p->business));
        setItem(row,ColBusiness,it);

        setItem(row,ColTrack,new SI(p->track));
        setItem(row,ColNote,new SI(p->note));

        //停时那一列，如果没有就不设置Item!
        int sec=p->stopSec();
        if(sec){
            it=new SI(p->stopString());
            setItem(row,ColStopTime,it);
        }
    }
}

void TimetableStdModel::actCancel()
{
}

void TimetableStdModel::actRemove()
{
}

void TimetableStdModel::actApply()
{

}
