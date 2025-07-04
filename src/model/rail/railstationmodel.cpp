﻿#include "railstationmodel.h"
#include "model/delegate/qedelegate.h"
#include "data/rail/railway.h"
#include "util/utilfunc.h"
#include <QMessageBox>
#include <QWidget>
#include <QFile>


RailStationModel::RailStationModel(bool inplace, QWidget *parent):
    QEMoveableModel(parent),commitInPlace(inplace)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({ "站名","里程","对里程","等级","显示","单向站", "单线",
        "办客","办货" });
    setupModel();
}

RailStationModel::RailStationModel(std::shared_ptr<Railway> rail,
                                   bool inplace, QWidget *parent):
    QEMoveableModel(parent),commitInPlace(inplace)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({ "站名","里程","对里程","等级","显示","单向站", "单线",
        "办客","办货" });
    setRailway(rail);
}


void RailStationModel::setRailway(std::shared_ptr<Railway> rail)
{
    railway = rail;
    setupModel();
}

void RailStationModel::setRailwayForDir(std::shared_ptr<Railway> rail, Direction dir)
{
    railway=rail;
    beginResetModel();
    if(railway){
        setRowCount(rail->stationCount());
        int row=0;
        for(auto p=rail->firstDirStation(dir);p;p=p->dirAdjacent(dir)){
            setStationRow(row++,p);
        }
        setRowCount(row);
    }else{
        setRowCount(0);
    }

    endResetModel();
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

    it = makeCheckItem();
    it->setCheckState(Qt::Unchecked);
    setItem(i, ColSingle, it);

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

QVariant RailStationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::ToolTipRole) {
            switch (section)
            {
            case RailStationModel::ColName:
                break;
            case RailStationModel::ColMile:
                break;
            case RailStationModel::ColCounter:return tr("上行方向的里程标，可选");
                break;
            case RailStationModel::ColLevel:
                break;
            case RailStationModel::ColShow:return tr("是否在运行图中显示车站水平线");
                break;
            case RailStationModel::ColDir:
                break;
            case RailStationModel::ColSingle:return tr("指定当前车站的前一个区间是否为单线区间");
                break;
            case RailStationModel::ColPassenger:
                break;
            case RailStationModel::ColFreight:
                break;
            case RailStationModel::ColMAX:
                break;
            default:
                break;
            }
        }
    }
    return QEMoveableModel::headerData(section, orientation, role);
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

    // 2022.09.11 新增单调性检查
    auto it = std::is_sorted_until(rail->stations().begin(), rail->stations().end(), 
        RailStationMileLess());
    if (it != rail->stations().end()) {
        QMessageBox::warning(par, tr("错误"), tr("线路里程标不满足单调递增要求，"
            "请检查[%1]站附近 （第%2行）").arg((*it)->name.toSingleLiteral())
            .arg(std::distance(rail->stations().begin(), it) + 1));
        return false;
    }

    //主要是检查每个区间里程非负；2025.07.04: we now require the interval mile to be positive (zero is invalid!!)
    for (auto p = rail->firstDownInterval(); p; p = rail->nextIntervalCirc(p)) {
        if (p->mile() <= 0) {
            QMessageBox::warning(par, tr("错误"),
                tr("区间距离为负数或为0，请重新设置里程！\n%1->%2区间，里程%3。")
                .arg(p->fromStationNameLit()).arg(p->toStationNameLit()).arg(p->mile()));
            return false;
        }
        else if (p->direction() == Direction::Down && p->downAdjacentStation()->prevSingle) {
            if (!p->isSymmetryInterval()) {
                // 非法单线区间，终止输入
                QMessageBox::warning(par, tr("错误"),
                    tr("单线区间设置不自洽错误：\n区间[%1->%2]非对称区间，但区间后站[%2]被设置为"
                        "单线区间的后站。请注意只有发、到站都是上下行通过站的区间，"
                        "才可以是单线区间。").arg(p->fromStationNameLit(), p->toStationNameLit()));
                return false;
            }
            else {
                // 合法单线区间，检查里程
                double mile_down = p->mile();
                double mile_up = p->inverseInterval()->mile();
                if (std::abs(mile_down - mile_up) > 1e-5) {
                    QMessageBox::warning(par, tr("错误"),
                        tr("单线区间里程不自洽：\n"
                            "区间[%1->%2]被设置为单线区间，但其上下行里程不相等。"
                            "下行里程: %3 km; 上行里程: %4 km。"
                            "\n请考虑调整"
                            "[对里程]设置项。").arg(p->fromStationNameLit(),p->toStationNameLit())
                        .arg(mile_down).arg(mile_up));
                    return false;
                }
            }
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
        const auto& name_str = item(i, ColName)->text();
        auto name = StationName::fromSingleLiteral(name_str);
        if (name.empty() || rail->stationNameExisted(name)) {
            QMessageBox::warning(p, tr("错误"), 
                tr("站名为空或已存在：\n第%1行: %2").arg(i+1).arg(name.toSingleLiteral()));
            return nullptr;
        }
        else if (name_str.contains('|')) {
            QMessageBox::information(p, tr("提醒"),
                tr("站名包含垂直线|(U+007): 第%1行: %2\n"
                    "自1.1.3版本开始，垂直线符号在某些功能将被用于多车站分隔符，" 
                    "强烈不建议在站名中包含此符号，否则可能产生异常结果。\n" 
                    "本提示不影响当前结果的提交。").arg(i + 1).arg(name_str));
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
        bool single = item(i, ColSingle)->checkState();

        rail->appendStation(name, mile, level, counter, dir, show, passen, freigh, single);

    }
    return rail;
}

std::shared_ptr<const RailStation> RailStationModel::getRowStation(int row)
{
    const QVariant& v = item(row, ColName)->data(qeutil::RailStationRole);
    return qvariant_cast<std::shared_ptr<const RailStation>>(v);
}

//#include <data/rail/forbid.h>

bool RailStationModel::applyChange()
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
    //qDebug() << "AFTER EMIT";
    //qDebug() << railway.get() << ", " << railway->forbids().at(0)->railway().get() << Qt::endl;
    //railway->getForbid(0)->_show();
    return true;
}

#define CHECK_PARSE_SUCCESS(flag, field) if (! flag) { \
	qWarning() << "Line: " << n << "Cannot parse field " << field; \
	return -1;\
}

int RailStationModel::fromCsv(const QString& filename, QString& report)
{
    enum Columns {
        CsvColName = 0,
        CsvColMile,
        CsvColCounterMile,
        CsvColLevel,
        CsvColShow,
        CsvColDirection,
        CsvColSingleLine,
        CsvColPassenger,
        CsvColFreight,
        CsvColMAX
    };

    QFile file(filename);
    file.open(QFile::ReadOnly);
    if (!file.isOpen()) {
        qWarning() << "Open file " << filename << " failed";
        return 0;
    }
    QTextStream sin(&file);

    int cnt = 0;
    int n = 0;

    sin.readLine();

    QString line;
    while (!sin.atEnd()) {
        n++;
        sin.readLineInto(&line);
        auto sp = line.split(',');
        if (sp.size() < ColMAX) {
            qWarning() << "Invalid line " << n << "; possibly file generated by old programs";
            return -1;
        }

        bool ok = true;
        StationName station_name(sp.at(CsvColName));
        double mile = sp.at(CsvColMile).toDouble(&ok);
        if (!ok) {
            report += tr("第%1行：无法解析的里程").arg(n);
            continue;
        }

        //RailStation st(station_name, mile);
        auto st = std::make_shared<RailStation>(station_name, mile);
        double counter_mile = sp.at(CsvColCounterMile).toDouble(&ok);
        if (ok) {
            st->counter = counter_mile;
        }

        st->level = sp.at(CsvColLevel).toInt();
        st->_show = (bool)sp.at(CsvColShow).toInt();
        int dir_idx = sp.at(CsvColDirection).toInt();
        if (dir_idx < 0 || dir_idx >= 4) {
            qWarning() << "Line: " << n << ": Invalid direction index: " << dir_idx;
            report += tr("第%1行：非法的“单向站”序号%2").arg(n).arg(dir_idx);
            continue;
        }
        st->direction = static_cast<PassedDirection>(dir_idx);

        st->prevSingle = (bool)sp.at(CsvColSingleLine).toInt();
        st->passenger = (bool)sp.at(CsvColPassenger).toInt();
        st->freight = (bool)sp.at(CsvColFreight).toInt();

        cnt++;
        setRowCount(cnt);
        setStationRow(cnt-1, st);
    }
    return cnt;
}

void RailStationModel::actApply()
{
    applyChange();
}

void RailStationModel::actCancel()
{
    setupModel();
}

bool RailStationModel::submit()
{
    emit dataSubmitted();
    return QEMoveableModel::submit();
}

void RailStationModel::setupModel()
{
    if (!railway) {
        setRowCount(0);
        return;
    }

    beginResetModel();

    setRowCount(railway->stationCount());

    for (int i = 0; i < rowCount(); i++) {
        //站名
        auto st = railway->stations().at(i);
        setStationRow(i,st);
    }
    endResetModel();
}

void RailStationModel::setStationRow(int i, std::shared_ptr<const RailStation> st)
{
    QStandardItem* item = new QStandardItem(st->name.toSingleLiteral());
    QVariant v;
    v.setValue(st);
    item->setData(v,qeutil::RailStationRole);
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

    // 单线区间
    item = makeCheckItem();
    item->setCheckState(st->prevSingle ? Qt::Checked : Qt::Unchecked);
    setItem(i, ColSingle, item);

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


