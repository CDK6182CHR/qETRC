#include "edgedatamodels.h"
#include "util/utilfunc.h"
#include "data/diagram/diagramoptions.h"


RulerNodesModel::RulerNodesModel(QObject *parent):
    QStandardItemModel(parent)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({tr("标尺名"),tr("通通"),tr("起"),tr("停"),
                               tr("运行速度")});
}

void RulerNodesModel::setupModel(std::shared_ptr<RailNet::edge> ed)
{
    using SI=QStandardItem;
    if(!ed){
        setRowCount(0);
        return;
    }
    setRowCount(ed->data.rulerNodes.size());
    for(int i=0;i<rowCount();i++){
        const auto& n=ed->data.rulerNodes.at(i);
        setItem(i,ColName,new SI(n.name));

        auto* it=new SI;
        it->setData(n.interval, Qt::EditRole);
        setItem(i,ColPass,it);
        it=new SI;
        it->setData(n.start,Qt::EditRole);
        setItem(i,ColStart,it);
        it=new SI;
        it->setData(n.stop,Qt::EditRole);
        setItem(i,ColStop,it);

        double spd=ed->data.mile/n.interval*3600;
        it=new SI;
        it->setData(spd,Qt::EditRole);
        setItem(i,ColSpeed,it);
    }
}



ForbidNodesModel::ForbidNodesModel(const DiagramOptions& ops, QObject *parent):
	QStandardItemModel(parent), _ops(ops)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({tr("开始"),tr("结束"),tr("时长")});
}

void ForbidNodesModel::setupModel(std::shared_ptr<RailNet::edge> ed)
{
    using SI=QStandardItem;
    if(!ed){
        setRowCount(0);
        return;
    }
    setRowCount(ed->data.forbidNodes.size());
    for(int i=0;i<rowCount();i++){
        const auto& n=ed->data.forbidNodes.at(i);
        auto* it=new SI;
        it->setData(QVariant::fromValue(n.beginTime), Qt::EditRole);
        setItem(i,ColBegin,it);
        it=new SI;
        it->setData(QVariant::fromValue(n.endTime), Qt::EditRole);
        setItem(i,ColEnd,it);
        setItem(i,ColDuration,new SI(qeutil::minsToStringHM(
                    qeutil::secsTo(n.beginTime, n.endTime, _ops.period_hours)/60)));
    }
}
