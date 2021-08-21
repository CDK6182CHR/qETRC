#include "intervaldatamodel.h"

IntervalDataModel::IntervalDataModel(Railway& railway, QObject *parent) :
    QStandardItemModel(parent),_railway(std::ref(railway))
{

}

void IntervalDataModel::setupModel()
{
    beginResetModel();
    updating=true;
    auto& rail=railway();
    setRowCount(rail.stationCount()*2);
    int row=0;
    for(auto p=rail.firstDownInterval();p;p=rail.nextIntervalCirc(p)){
        setupRow(row++,p);
    }
    setRowCount(row);
    endResetModel();
    updating=false;
}

void IntervalDataModel::setupRow(int row, std::shared_ptr<RailInterval> railint)
{
    auto* it=new QStandardItem(intervalString(*railint));
    it->setEditable(false);
    setItem(row,0,it);
}

QString IntervalDataModel::intervalString(const RailInterval& railint)const
{
    return tr("%1->%3").arg(railint.fromStationNameLit(),railint.toStationNameLit());
}

void IntervalDataModel::copyRowData(int from, int to)
{

}

bool IntervalDataModel::checkRowInterval(const RailInterval& railint, int row) const
{
    if (intervalString(railint) == item(row, 0)->text()) {
        return true;
    }
    else {
        qDebug() << "IntervalDataModel::checkRowInterval: WARNING: " <<
            "Interval not complicated: expected " << item(row, 0)->text() << ", " <<
            "get " << railint << Qt::endl;
        return false;
    }
}

bool IntervalDataModel::checkRowInterval(std::shared_ptr<RailInterval> railint, int row) const
{
    return checkRowInterval(*railint, row);
}

void IntervalDataModel::copyFromDownToUp()
{
    //正向遍历，所有的下行区间打表
    //暂定直接用字符串比较的方法确定是否匹配
    QMap<QPair<std::shared_ptr<RailStation>,std::shared_ptr<RailStation>>,int> itrows;

    int row=0;
    for(auto p=railway().firstDownInterval();p;
        p=p->nextInterval()){
        if (checkRowInterval(p,row)){
            itrows.insert(qMakePair(p->fromStation(),p->toStation()),row);
        }
        row++;
    }

    //上行：接着读
    for(auto p=railway().firstUpInterval();p;p=p->nextInterval()){
        if (checkRowInterval(p,row)){
            auto&& key=qMakePair(p->toStation(),p->fromStation());
            if(itrows.contains(key)){
                copyRowData(itrows.value(key),row);
            }
        }
        row++;
    }
}

void IntervalDataModel::copyFromUpToDown()
{
    //这里仍然记得是下行数据
    QMap<QPair<std::shared_ptr<RailStation>,std::shared_ptr<RailStation>>,int> itrows;
    int row=0;
    for(auto p=railway().firstDownInterval();p;p=p->nextInterval()){
        if(checkRowInterval(p,row)){
            itrows.insert(qMakePair(p->fromStation(),p->toStation()),row);
        }
        row++;
    }

    //上行
    for(auto p=railway().firstUpInterval();p;p=p->nextInterval()){
        if(checkRowInterval(p,row)){
            auto&& key=qMakePair(p->toStation(),p->fromStation());
            if(itrows.contains(key)){
                copyRowData(row,itrows.value(key));
            }
        }
    }
}

