#include "timetablequickmodel.h"
#include "data/train/train.h"
#include "util/utilfunc.h"
#include "model/delegate/qedelegate.h"
#include "data/diagram/trainadapter.h"

TimetableQuickModel::TimetableQuickModel(QObject *parent) : QStandardItemModel(parent)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({tr("站名"),tr("时刻"),tr("停时股道")});
}

void TimetableQuickModel::setupModel()
{
    if (!train) {
        setRowCount(0);
        return;
    }
    updating = true;
    train->refreshStationFlags();
    using SI = QStandardItem;
    setRowCount(2 * train->timetable().size());
    int row = 0;
    for (auto p = train->timetable().begin(); p != train->timetable().end(); ++p) {
        if (p->isStopped()) {
            //停车的情况
            setTimeItem(row, p->arrive);
            setTimeItem(row + 1, p->depart);
        }
        else if (train->isStartingStation(p->name)) {
            //标准始发站情况（停时为0）
            setTimeItem(row, p->arrive, "");
            setTimeItem(row + 1, p->depart);
        }
        else if (train->isTerminalStation(p->name)) {
            //标准终到站
            setTimeItem(row, p->arrive);
            setTimeItem(row + 1, p->depart, "--");
        }
        else {
            //通过站
            setTimeItem(row, p->arrive, "...");
            setTimeItem(row + 1, p->depart);
        }
        setupStationExceptTime(p, row);

        row += 2;
    }
    setupBoudingMap();
    updating = false;
}

bool TimetableQuickModel::isAlwaysShow(int row) const
{
    auto p = qvariant_cast<Train::StationPtr>(
        item(row, ColName)->data(qeutil::TrainStationRole));
    return p->flag & TrainStation::NonPass;
}

TrainStationBoundingList TimetableQuickModel::getBoundingList(int row) const
{
    if (row % 2 != 0)
        --row;
    auto st = trainStationForRow(row);
    if (auto itr = boundingMap.find(&*st); itr != boundingMap.end()) {
        return itr->second;
    }
    else {
        return {};
    }
}

std::list<TrainStation>::iterator TimetableQuickModel::trainStationForRow(int row)const
{
    return qvariant_cast<Train::StationPtr>(
        item(row, ColName)->data(qeutil::TrainStationRole));
}

QTime TimetableQuickModel::arriveTimeForRow(int row) const
{
    if (row % 2 != 0)row--;
    return item(row, ColTime)->data(qeutil::TimeDataRole).toTime();
}

QTime TimetableQuickModel::departTimeForRow(int row) const
{
    if (row % 2 != 0)row++;
    return item(row, ColTime)->data(qeutil::TimeDataRole).toTime();
}

void TimetableQuickModel::setTimeItem(int row, const QTime &tm)
{
    auto* it = new QStandardItem(tm.toString("hh:mm:ss"));
    it->setData(tm, qeutil::TimeDataRole);
    setItem(row,ColTime,it);
}

void TimetableQuickModel::setTimeItem(int row, const QTime& tm, const QString& text)
{
    auto* it = new QStandardItem(text);
    it->setData(tm, qeutil::TimeDataRole);
    setItem(row, ColTime, it);
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

void TimetableQuickModel::setupBoudingMap()
{
    boundingMap.clear();
    foreach(auto adp, train->adapters()) {
        foreach(auto line, adp->lines()) {
            for (const auto& p : line->stations()) {
                boundingMap[&*p.trainStation].push_back(
                    TrainStationBounding(line, p.railStation));
            }
        }
    }
}

QStandardItem* TimetableQuickModel::NESI(const QString& text)
{
    auto* it = new QStandardItem(text);
    it->setEditable(false);
    return it;
}

QStandardItem* TimetableQuickModel::NESI()
{
    auto* it = new QStandardItem;
    it->setEditable(false);
    return it;
}

void TimetableQuickModel::setStationStopCol(const TrainStation& station, int row)
{
    QColor color(Qt::blue);
    if (station.isStopped()) {
        //停车的情况
        setItem(row, ColNote, NESI(qeutil::secsToString(station.stopSec())));
    }
    else if (train->isStartingStation(station.name)) {
        //标准始发站情况（停时为0）
        setItem(row, ColNote, NESI(tr("始")));
    }
    else if (train->isTerminalStation(station.name)) {
        //标准终到站
        setItem(row, ColNote, NESI(tr("终")));
    }
    else {
        //通过站
        color = Qt::black;
        takeItem(row, ColNote);
    }
    if (station.flag & TrainStation::Bound) {
        if (station.flag & TrainStation::Business &&
            station.flag & TrainStation::NonPass) {
            color = Qt::red;
        }
    }
    else {
        color = Qt::darkGray;
    }
    setItem(row + 1, ColNote, NESI(station.track));
    setStationColor(row, color);
}

void TimetableQuickModel::setupStationExceptTime(std::list<TrainStation>::iterator  p,
    int row)
{
    auto* it = NESI(p->name.toSingleLiteral());
    it->setEditable(false);
    QVariant v;
    v.setValue(p);
    it->setData(v, qeutil::TrainStationRole);
    setItem(row, ColName, it);

    setStationStopCol(*p, row);
}


void TimetableQuickModel::setTrain(std::shared_ptr<Train> train)
{
    this->train=train;
    setupModel();
}

TimetableQuickEditableModel::TimetableQuickEditableModel(QUndoStack* undo_, QObject* parent):
    TimetableQuickModel(parent),_undo(undo_)
{
    connect(this, &QStandardItemModel::dataChanged,
        this, &TimetableQuickEditableModel::onDataChanged);
}

void TimetableQuickEditableModel::updateStationTime(const TrainStation& station, int row)
{
    updating = true;
    if (station.isStopped()) {
        //停车的情况
        setTimeItem(row, station.arrive);
        setTimeItem(row + 1, station.depart);
    }
    else if (train->isStartingStation(station.name)) {
        //标准始发站情况（停时为0）
        setTimeItem(row, station.arrive, "");
        setTimeItem(row + 1, station.depart);
    }
    else if (train->isTerminalStation(station.name)) {
        //标准终到站
        setTimeItem(row, station.arrive);
        setTimeItem(row + 1, station.depart, "--");
    }
    else {
        //通过站
        setTimeItem(row, station.arrive, "...");
        setTimeItem(row + 1, station.depart);
    }
    setStationStopCol(station, row);   // 这里包含一点重复的操作  Track其实不用重设
    updating = false;
}

void TimetableQuickEditableModel::onStationTimeChanged(int row)
{
    int st_row = row;
    if (row % 2 != 0)st_row--;
    auto p = qvariant_cast<Train::StationPtr>(item(st_row, ColName)->data(qeutil::TrainStationRole));
    TrainStation st = TrainStation(*p);    // copy contruct
    
    
    //st.arrive = item(row, ColTime)->data(qeutil::TimeDataRole).toTime();
    //st.depart = item(row + 1, ColTime)->data(qeutil::TimeDataRole).toTime();
    //st.updateStopFlag();
    const auto& tm = item(row, ColTime)->data(qeutil::TimeDataRole).toTime();;

    //注意：如果编辑的是非停车站有数据的那个点，那么认为另一个点跟着动了。
    if (p->flag & TrainStation::Stopped) {
        // 停车的情况，直接更新
        if (row % 2 == 0)   // 偶数行，到达
            st.arrive = tm;
        else
            st.depart = tm;
        st.updateStopFlag();    //有可能变为不停车，所以要更新
    }
    else if (p->flag & TrainStation::Terminal) {
        // 不停车，但是终到，则上面那个数据是有效的
        if (row % 2 == 0) {
            // 更改的是有效数据，则视作一起改
            st.arrive = st.depart = tm;
        }
        else {
            st.depart = tm;
            st.updateStopFlag();
        }
    }
    else {
        // 剩下的情况：通过或者始发，下面那个数据有效
        if (row % 2 != 0) {
            // 更改的是有效数据，一起改
            st.arrive = st.depart = tm;
        }
        else {
            st.arrive = tm;
            st.updateStopFlag();
        }
    }

    _undo->push(new qecmd::AdjustTrainStationTime(train, st_row, std::move(st), this,
        p->flag & TrainStation::Bound));
}

void TimetableQuickEditableModel::onDataChanged(const QModelIndex& topLeft, 
    const QModelIndex& bottomRight, const QVector<int>& roles)
{
    if (updating)
        return;
    if (roles.contains(qeutil::TimeDataRole)) {
        if (topLeft.column() <= ColTime && ColTime <= bottomRight.column()) {
            for (int i = topLeft.row(); i <= bottomRight.row(); i++) {
                // 一般情况下，不认为会发生多于一行的变化。
                onStationTimeChanged(i);
            }
        }
    }
}

void TimetableQuickEditableModel::commitStationTimeChange(std::shared_ptr<Train> train,
    int row, TrainStation& data, bool repaint)
{
    if (train == this->train) {
        // 当前列车没变，更新表格
        auto st = qvariant_cast<Train::StationPtr>(
            item(row, ColName)->data(qeutil::TrainStationRole));
        std::swap(*st, data);
        updateStationTime(*st, row);
    }
    else {
        // 暂定采用线性算法定位到车站。注意2倍
        auto p = train->timetable().begin();
        std::advance(p, row / 2);   // int div
        std::swap(*p, data);
    }
    emit trainStationTimeUpdated(train, repaint);
}

qecmd::AdjustTrainStationTime::AdjustTrainStationTime(std::shared_ptr<Train> train_,
    int row_, TrainStation&& data_, TimetableQuickEditableModel* const model_,
    bool repaint_,
    QUndoCommand* parent):
    QUndoCommand(QObject::tr("微调时刻: [%1]站 @ %2")
        .arg(data_.name.toSingleLiteral(),train_->trainName().full()),parent),
    train(train_),
    row(row_),data(std::forward<TrainStation>(data_)),model(model_),repaint(repaint_)
{
}

void qecmd::AdjustTrainStationTime::undo()
{
    model->commitStationTimeChange(train, row, data, repaint);
}
void qecmd::AdjustTrainStationTime::redo()
{
    model->commitStationTimeChange(train, row, data, repaint);
}



bool qecmd::AdjustTrainStationTime::mergeWith(const QUndoCommand* other)
{
    if (id() != other->id())return false;
    auto cmd = static_cast<const AdjustTrainStationTime*>(other);
    if (train == cmd->train && row == cmd->row) {
        // 抛弃中间状态即可
        return true;
    }
    else return false;
}
