#include "timetablestdmodel.h"
#include <utility>
#include <QMessageBox>

#include "util/utilfunc.h"


TimetableStdModel::TimetableStdModel(bool inplace, QWidget *parent):
    QEMoveableModel(parent), commitInPlace(inplace)
{
    connect(this, &QStandardItemModel::dataChanged, this, &TimetableStdModel::onDataChanged);
    setHorizontalHeaderLabels({
        tr("站名"),tr("到点"),tr("开点"),tr("营业"),tr("股道"),tr("备注"),tr("停时")
        });
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
    updating = true;
    using SI=QStandardItem;
    setRowCount(static_cast<int>( _train->timetable().size()));
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
            it->setEditable(false);
            setItem(row,ColStopTime,it);
        }
        else {
            takeItem(row, ColStopTime);
        }
    }
    updating = false;
}

std::shared_ptr<Train> TimetableStdModel::getTimetableTrain()
{
    auto* p = qobject_cast<QWidget*>(parent());
    auto t = std::make_shared<Train>(_train->trainName());
    for (int i = 0; i < rowCount(); i++) {
        const  QString& name = item(i, ColName)->text();
        if (name.isEmpty()) {
            QMessageBox::warning(p, tr("错误"),
                tr("站名不能为空，请重新输入。\n第%1行").arg(i + 1));
            return nullptr;
        }
        t->appendStation(
            StationName::fromSingleLiteral(name),
            qvariant_cast<QTime>(item(i, ColArrive)->data(Qt::EditRole)),
            qvariant_cast<QTime>(item(i, ColDepart)->data(Qt::EditRole)),
            item(i, ColBusiness)->checkState() == Qt::Checked,
            item(i, ColTrack)->text(),
            item(i, ColNote)->text()
        );
    }
    return t;
}

void TimetableStdModel::updateRowStopTime(int row)
{
    const QTime& arr = qvariant_cast<QTime>(item(row, ColArrive)->data(Qt::EditRole));
    const QTime& dep = qvariant_cast<QTime>(item(row, ColDepart)->data(Qt::EditRole));
    if (arr != dep) {
        auto* it = new QStandardItem(qeutil::secsToString(arr, dep));
        it->setEditable(false);
        setItem(row, ColStopTime, it);
    }
    else {
        takeItem(row, ColStopTime);
    }
}

void TimetableStdModel::actCancel()
{
    refreshData();
}

void TimetableStdModel::onDataChanged(const QModelIndex& leftTop, const QModelIndex& rightBottom)
{
    if (updating)
        return;
    int row1 = leftTop.row(), row2 = rightBottom.row();
    int col1 = leftTop.column(), col2 = rightBottom.column();
    //[row1,row2]包含时间的两列条件
    if (std::max(col1, static_cast<int>(ColArrive)) <= 
        std::min(col2, static_cast<int>(ColDepart))) {
        for (int i = std::max(row1,0); i <= std::min(row2,rowCount()-1); i++) {
            updateRowStopTime(i);
        }
    }
}

void TimetableStdModel::actApply()
{
    auto t = getTimetableTrain();
    if (!t)
        return;
    if (_train->timetableSame(*t))
        return;
    emit timetableChanged(_train, t);
}
