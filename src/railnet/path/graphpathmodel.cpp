#include "graphpathmodel.h"

GraphPathModel::GraphPathModel(QObject *parent) : QStandardItemModel(parent)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({tr("站名"),tr("里程"),tr("出度"),tr("等级")});
}

void GraphPathModel::setPath(const RailNet::path_t &path)
{
    this->path=path;
    setupModel();
}

void GraphPathModel::setPath(RailNet::path_t &&path)
{
    this->path=std::forward<RailNet::path_t>(path);
    setupModel();
}

RailNet::path_t GraphPathModel::subPathTo(int row)
{
    if (row<=0) return {};
    RailNet::path_t res;
    res.insert(res.end(),path.begin(),path.begin()+row);
    return res;
}

void GraphPathModel::setupModel()
{
    if (path.empty()){
        setRowCount(0);
        return;
    }
    setRowCount(path.size()+1);
    int row=0;
    double mile=0;
    setupRow(row++,mile, path.front()->from.lock().get());

    for(const auto& e: path){
        setupRow(row++,(mile+=e->data.mile),e->to.lock().get());
    }
}

void GraphPathModel::setupRow(int row, double mile, const RailNet::vertex *v)
{
    using SI=QStandardItem;
    setItem(row,ColName,new SI(v->data.name.toSingleLiteral()));
    setItem(row,ColMile,new SI(QString::number(mile,'f',3)));
    setItem(row,ColOutDeg,new SI(QString::number(v->out_degree())));
    setItem(row,ColLevel,new SI(QString::number(v->data.level)));
}
