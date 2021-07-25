#include "railstationmodel.h"

#include <QtWidgets>


RailStationModel::RailStationModel(bool inplace, QWidget *parent):
    QEMoveableModel(parent),commitInPlace(inplace)
{
    setupModel();
}

RailStationModel::RailStationModel(std::shared_ptr<Railway> rail,
                                   bool inplace, QWidget *parent):
    QEMoveableModel(parent),commitInPlace(inplace)
{
    setRailway(rail);
}


void RailStationModel::setRailway(std::shared_ptr<Railway> rail)
{
    railway = rail;
    setupModel();
}

void RailStationModel::setupNewRow(int i)
{
    using SI = QStandardItem;

    setItem(i, ColName, new QStandardItem);

    auto* it = new SI;
    it->setData(0.0, Qt::EditRole);
    setItem(i, ColMile, it);

    setItem(i, ColCounter, new SI);

    it = new SI;
    it->setData(4, Qt::EditRole);
    setItem(i, ColLevel, it);

    it = new SI;
    it->setCheckState(Qt::Checked);
    it->setCheckable(true);
    it->setEditable(false);
    setItem(i, ColShow, it);

    it = new SI;
    it->setData(3, Qt::EditRole);
    setItem(i, ColDir, it);

    it = new SI;
    it->setCheckState(Qt::Unchecked);
    it->setCheckable(true);
    it->setEditable(false);
    setItem(i, ColPassenger, it);

    it = new SI;
    it->setCheckState(Qt::Unchecked);
    it->setEditable(false);
    it->setCheckable(true);
    setItem(i, ColFreight, it);
}


QVariant RailStationModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && index.column() == ColDir && role == Qt::DisplayRole) {
        return qeutil::passDirStr(index.data(Qt::EditRole).toInt());
    }
    return QStandardItemModel::data(index, role);
}

Qt::ItemFlags RailStationModel::flags(const QModelIndex& index) const
{
    return QStandardItemModel::flags(index);
}


bool RailStationModel::checkRailway(std::shared_ptr<Railway> rail)
{
    auto* par = qobject_cast<QWidget*>(parent());
    if (rail->empty())return true;
    else if (rail->stations().first()->direction != PassedDirection::BothVia ||
        rail->stations().last()->direction != PassedDirection::BothVia)
    {
        QMessageBox::warning(par, tr("错误"),
            tr("第一站和最后一站必须设置为“双向通过”，请重新设置！"));
        return false;
    }

    //主要是检查每个区间里程非负
    for (auto p = rail->firstDownInterval(); p; p = rail->nextIntervalCirc(p)) {
        if (p->mile() < 0) {
            QMessageBox::warning(par, tr("错误"),
                tr("区间距离为负数，请重新设置里程！\n%1->%2区间，里程%3。")
                .arg(p->fromStationNameLit()).arg(p->toStationNameLit()).arg(p->mile()));
            return false;
        }
    }

    auto first = rail->stations().first();
    if (first->mile != 0 || first->counter.value_or(0) != 0) {
        auto res = QMessageBox::question(par, tr("提示"), tr("本线首站里程或对里程不为0。"
            "是否要调整所有站里程，以使得首站里程归零？"));
        if (res == QMessageBox::Yes)
            rail->adjustMileToZero();
    }
    return true;
}

std::shared_ptr<Railway> RailStationModel::generateRailway() const
{
    auto rail = std::make_shared<Railway>(railway->name());
    QWidget* p = qobject_cast<QWidget*>(parent());
    for (int i = 0; i < rowCount(); i++) {
        auto name = StationName::fromSingleLiteral(item(i, ColName)->text());
        if (name.empty() || rail->stationNameExisted(name)) {
            QMessageBox::warning(p, tr("错误"), 
                tr("站名为空或已存在：\n第%1行: %2").arg(i+1).arg(name.toSingleLiteral()));
            return nullptr;
        }

        double mile = qvariant_cast<double>(item(i, ColMile)->data(Qt::EditRole));

        bool ok;
        std::optional<double> counter = item(i, ColCounter)->data(Qt::EditRole).toDouble(&ok);
        if (!ok)
            counter = std::nullopt;

        int level = qvariant_cast<int>(item(i, ColLevel)->data(Qt::EditRole));
        bool show = item(i, ColShow)->checkState();
        PassedDirection dir = static_cast<PassedDirection>(
            qvariant_cast<int>(item(i, ColDir)->data(Qt::EditRole)));
        bool passen = item(i, ColPassenger)->checkState();
        bool freigh = item(i, ColFreight)->checkState();

        rail->appendStation(name, mile, level, counter, dir, show, passen, freigh);

    }
    return rail;
}

bool RailStationModel::actApply()
{
    auto rail = generateRailway();
    if (!rail)
        return false;
    if (!checkRailway(rail))
        return false;
    bool equiv = rail->mergeIntervalData(*railway);

    if (commitInPlace) {
        //不支持撤销，通常也不需要通告别人 
        //目前没有这种情况，需要的时候再设计！
        railway->swapBaseWith(*rail);
        refreshData();
    }
    else {
        //支持撤销，则交给Context处理
        emit actStationTableChanged(railway, rail, equiv);
    }
    return false;
}

void RailStationModel::actCancel()
{
    setupModel();
}

void RailStationModel::setupModel()
{
    beginResetModel();
    setColumnCount(ColMAX);
    if (!railway) {
        setRowCount(0);
        return;
    }

    setRowCount(railway->stationCount());

    setHorizontalHeaderLabels({ "站名","里程","对里程","等级","显示","单向站",
        "办客","办货" });

    for (int i = 0; i < rowCount(); i++) {
        //站名
        auto st = railway->stations().at(i);
        QStandardItem* item = new QStandardItem(st->name.toSingleLiteral());
        setItem(i, ColName, item);

        //里程
        item = new QStandardItem(QString::number(st->mile, 'f', 3));
        item->setData(st->mile, Qt::EditRole);
        setItem(i, ColMile, item);

        //对里程
        item = new QStandardItem;
        if (st->counter.has_value())
            item->setData(QString::number(st->counter.value(), 'f', 3), Qt::DisplayRole);
        else
            item->setData("", Qt::DisplayRole);
        setItem(i, ColCounter, item);

        //等级
        item = new QStandardItem(QString::number(st->level));
        item->setData(st->level, Qt::EditRole);
        setItem(i, ColLevel, item);

        //显示
        item = new QStandardItem;
        item->setCheckState(st->_show ? Qt::Checked : Qt::Unchecked);
        item->setCheckable(true);
        item->setEditable(false);
        setItem(i, ColShow, item);

        //单向站
        item = new QStandardItem;
        item->setData(static_cast<int>(st->direction), Qt::EditRole);
        setItem(i, ColDir, item);

        //办客
        item = new QStandardItem;
        item->setCheckable(true);
        item->setEditable(false);
        item->setCheckState(qeutil::boolToCheckState(st->passenger));
        setItem(i, ColPassenger, item);

        //办货
        item = new QStandardItem;
        item->setCheckable(true);
        item->setEditable(false);
        item->setCheckState(qeutil::boolToCheckState(st->freight));
        setItem(i, ColFreight, item);
    }
    endResetModel();
}


