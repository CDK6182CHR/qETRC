#include "adjacentlistmodel.h"
#include "model/delegate/qedelegate.h"


AdjacentListModel::AdjacentListModel(const RailNet &net, QObject *parent):
    QStandardItemModel(parent), net(net)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({tr("线名"),tr("自"),tr("至"),tr("方向"),tr("里程"),
                               tr("标尺数")});
}

std::shared_ptr<RailNet::edge> AdjacentListModel::edgeFromRow(int row)
{
    if (row >= 0)
        return qvariant_cast<std::shared_ptr<RailNet::edge>>(
            item(row, ColRailName)->data(qeutil::GraphEdgeRole));
    else return nullptr;
}

void AdjacentListModel::setupForInAdj(std::shared_ptr<const RailNet::vertex> v)
{
    ve=v;
    if(!ve){
        setRowCount(0);
        return;
    }
    setRowCount(v->in_degree());
    int row=0;
    for(auto e=v->in_edge;e;e=e->next_in){
        setupRow(e,row++);
    }
}

void AdjacentListModel::setupForOutAdj(std::shared_ptr<const RailNet::vertex> v)
{
    ve=v;
    if(!ve){
        setRowCount(0);
        return;
    }
    setRowCount(v->out_degree());
    int row=0;
    for(auto e=v->out_edge;e;e=e->next_out){
        setupRow(e,row++);
    }
}

void AdjacentListModel::setupRow(std::shared_ptr<RailNet::edge> e, int row)
{
    using SI=QStandardItem;

    auto* it=new SI(e->data.railName);
    QVariant v;
    v.setValue(e);
    it->setData(v,qeutil::GraphEdgeRole);
    setItem(row,ColRailName,it);

    setItem(row,ColFrom,new SI(e->from.lock()->data.name.toSingleLiteral()));
    setItem(row,ColTo,new SI(e->to.lock()->data.name.toSingleLiteral()));
    setItem(row,ColDir,new SI(DirFunc::dirToString(e->data.dir)));

    it=new SI;
    it->setData(e->data.mile, Qt::EditRole);
    setItem(row,ColMile,it);
    it=new SI;
    it->setData(e->data.rulerNodes.size(),Qt::EditRole);
    setItem(row,ColRulerCount,it);
}
