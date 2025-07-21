#include "timetablestdmodel.h"
#include <utility>
#include <QMessageBox>

#include "util/utilfunc.h"
#include "data/train/train.h"
#include "util/utilfunc.h"
#include "data/diagram/diagramoptions.h"

TimetableStdModel::TimetableStdModel(const DiagramOptions& ops, bool inplace, QWidget *parent):
    QEMoveableModel(parent), _ops(ops), commitInPlace(inplace)
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
    it->setData(QVariant::fromValue(TrainTime{}), Qt::EditRole);
    setItem(row,ColArrive,it);

    it=new SI;
    it->setData(QVariant::fromValue(TrainTime{}),Qt::EditRole);
    setItem(row,ColDepart,it);

    setItem(row,ColBusiness,makeCheckItem());
    setItem(row,ColTrack,new SI);
    setItem(row,ColNote,new SI);

    it = new SI;
    it->setEditable(false);
    setItem(row, ColStopTime, it);
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
        auto* itname = new SI(p->name.toSingleLiteral());
        setItem(row, ColName, itname);

        auto* it=new SI;
        QVariant v1;
		v1.setValue(p->arrive);
        it->setData(v1,Qt::EditRole);
        setItem(row,ColArrive,it);

        it=new SI;
        QVariant v2;
		v2.setValue(p->depart); 
        it->setData(v2,Qt::EditRole);
        setItem(row,ColDepart,it);

        it=makeCheckItem();
        it->setCheckState(qeutil::boolToCheckState(p->business));
        setItem(row,ColBusiness,it);

        setItem(row,ColTrack,new SI(p->track));
        setItem(row,ColNote,new SI(p->note));

        //停时那一列，有没有都要弄个不可编辑的Item在那里
        it = new SI;
        it->setEditable(false);
        int sec=p->stopSec();
        if(sec){
            it->setText(p->stopString());
        }
        setItem(row, ColStopTime, it);

        // 2022.06.24  改为用统一方法设置颜色
        setRowColor(row);

        //if (p->business && sec) {
        //    itname->setForeground(Qt::red);
        //}
        //else if(sec){
        //    itname->setForeground(Qt::blue);
        //}
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
            qvariant_cast<TrainTime>(item(i, ColArrive)->data(Qt::EditRole)),
            qvariant_cast<TrainTime>(item(i, ColDepart)->data(Qt::EditRole)),
            item(i, ColBusiness)->checkState() == Qt::Checked,
            item(i, ColTrack)->text(),
            item(i, ColNote)->text()
        );
    }
    return t;
}

void TimetableStdModel::updateRowStopTime(int row)
{
    const TrainTime& arr = qvariant_cast<TrainTime>(item(row, ColArrive)->data(Qt::EditRole));
    const TrainTime& dep = qvariant_cast<TrainTime>(item(row, ColDepart)->data(Qt::EditRole));
    auto* it = item(row, ColStopTime);

    if (arr != dep) {
        it->setText(qeutil::secsToString(arr, dep, _ops.period_hours));
    }
    else {
        it->setText("");
    }
    setRowColor(row);
}

void TimetableStdModel::setRowColor(int row)
{
    auto* it = item(row, ColName);
    if (item(row, ColBusiness)->checkState() == Qt::Checked) {
        it->setForeground(Qt::red);
    }
    else {
        const QTime& arr = qvariant_cast<QTime>(item(row, ColArrive)->data(Qt::EditRole));
        const QTime& dep = qvariant_cast<QTime>(item(row, ColDepart)->data(Qt::EditRole));
        if (arr != dep) {
            it->setForeground(Qt::blue);
        }
        else {
            it->setForeground(Qt::black);
        }
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
    else if (col1 <= static_cast<int>(ColBusiness) && static_cast<int>(ColBusiness) <= col2) {
        for (int i = std::max(row1, 0); i <= std::min(row2, rowCount() - 1); i++) {
            setRowColor(i);
        }
    }
}

void TimetableStdModel::copyToDepart(int row)
{
    if (0 <= row && row < rowCount()) {
        item(row, ColDepart)->setData(item(row, ColArrive)->data(Qt::EditRole), Qt::EditRole);
    }
}

void TimetableStdModel::copyToArrive(int row)
{
    if (0 <= row && row < rowCount()) {
        item(row, ColArrive)->setData(item(row, ColDepart)->data(Qt::EditRole), Qt::EditRole);
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

TimetableConstModel::TimetableConstModel(QObject* parent):
    QStandardItemModel(parent)
{
    setHorizontalHeaderLabels({
    tr("站名"),tr("到点"),tr("开点"),tr("营业"),tr("股道"),tr("备注"),tr("停时")
        });
}

void TimetableConstModel::setTrain(std::shared_ptr<const Train> train)
{
    _train = train;
    setupModel();
}

void TimetableConstModel::refreshData()
{
    setupModel();
}

void TimetableConstModel::setupModel()
{
    if (!_train) {
        setRowCount(0);
        return;
    }
    using SI = QStandardItem;
    setRowCount(static_cast<int>(_train->timetable().size()));
    setColumnCount(ColMAX);
    int row = 0;

    //注意Timetable是std::list，不能用随机访问
    for (auto p = _train->timetable().begin(); p != _train->timetable().end();
        ++p, ++row)
    {
        auto* itname = new SI(p->name.toSingleLiteral());
        setItem(row, ColName, itname);

        auto* it = new SI;
        QVariant v1;
        v1.setValue(p->arrive);
        it->setData(v1, Qt::EditRole);
        setItem(row, ColArrive, it);

        it = new SI;
        QVariant v2;
        v2.setValue(p->depart);
        it->setData(v2, Qt::EditRole);
        setItem(row, ColDepart, it);

        it = makeCheckItem();
        it->setCheckState(qeutil::boolToCheckState(p->business));
        setItem(row, ColBusiness, it);

        setItem(row, ColTrack, new SI(p->track));
        setItem(row, ColNote, new SI(p->note));

        //停时那一列，如果没有就不设置Item!
        int sec = p->stopSec();
        if (sec) {
            it = new SI(p->stopString());
            it->setEditable(false);
            setItem(row, ColStopTime, it);
        }
        else {
            takeItem(row, ColStopTime);
        }

        if (p->business) {
            itname->setForeground(Qt::red);
        }
        else if (sec) {
            itname->setForeground(Qt::blue);
        }
    }
}

QStandardItem* TimetableConstModel::makeCheckItem()
{
    auto* it = new QStandardItem;
    it->setEditable(false);
    it->setCheckState(Qt::Unchecked);
    it->setCheckable(false);
    return it;
}



