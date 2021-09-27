#include "pathoperationmodel.h"


PathOperationModel::PathOperationModel(const RailNet &net, QObject *parent):
    QAbstractTableModel(parent), net(net)
{

}

int PathOperationModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (seq.empty())
         return 0;
     else return 1+seq.operations().size();
}

int PathOperationModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return ColMAX;
}

QVariant PathOperationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return {};
    if (role==Qt::DisplayRole || role==Qt::ToolTipRole){
        if (index.row()==0){
            switch (index.column())
            {
            case PathOperationModel::ColName: 
                return seq.start()->data.name.toSingleLiteral();
            case PathOperationModel::ColMile:
                return "0.000";
            case PathOperationModel::ColMethod:
                return tr("首站");
            case PathOperationModel::ColPath:
                return "";
            }
        }else{
            const auto& op = seq.operations().at(index.row() - 1);
            switch (index.column())
            {
            case PathOperationModel::ColName:
                return op.target->data.name.toSingleLiteral();
            case PathOperationModel::ColMile:
                return QString::number(op.mileStone, 'f', 3);
            case PathOperationModel::ColMethod:
                return PathOperation::methodString(op.method);
            case PathOperationModel::ColPath:
                return net.pathToStringSimple(op.path);
            }
        }
    }
    return {};
}

QVariant PathOperationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole
            && orientation == Qt::Horizontal) {
        switch (section)
        {
        case PathOperationModel::ColName:
            return tr("站名");
        case PathOperationModel::ColMile:
            return tr("里程");
        case PathOperationModel::ColMethod:
            return tr("添加方法");
        case PathOperationModel::ColPath:
            return tr("经由");
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

bool PathOperationModel::addByShortestPath(const QString &target, QString *report)
{
    auto v=net.find_vertex(target);
    if (!v){
        report->append(tr("所给车站%1不存在").arg(target));
        return false;
    }
    return addByShortestPath(v, report);
}

bool PathOperationModel::addByShortestPath(
        const std::shared_ptr<const RailNet::vertex> &target,
        QString *report)
{
    if (seq.empty()){
        // 首站
        addStartStation(target);
    }else{
        // 非首站：最短路算法
        auto path=net.shortestPath(seq.lastVertex(),target,report);
        if (path.empty()){
            return false;
        }
        else{
            appendOperation(PathOperation(target,PathOperation::ShortestPath,
                path,seq.currentMile()+RailNet::pathMile(path)));
        }
    }
    return true;
}

bool PathOperationModel::addByAdjStation(std::shared_ptr<RailNet::edge> ed, QString *report)
{
    if(seq.empty()){
        report->append(QObject::tr("起始站为空。请先选择起始站。"));
        return false;
    }
    if (seq.lastVertex()!=ed->from.lock()){
        report->append(QObject::tr("意外的邻接边：预期起始站%1，输入起始站%2").arg(
                           seq.lastVertex()->data.name.toSingleLiteral(),
                           ed->from.lock()->data.name.toSingleLiteral()));
        return false;
    }
    appendOperation(PathOperation(ed->to.lock(),PathOperation::AdjStation,
                                  {ed},seq.currentMile()+ed->data.mile));
    return true;
}

bool PathOperationModel::addByAdjPath(RailNet::path_t &&path, QString *report)
{
    if(seq.empty()){
        report->append(QObject::tr("起始站为空。请先选择起始站。"));
        return false;
    }
    if (seq.lastVertex()!=path.front()->from.lock()){
        report->append(QObject::tr("意外的邻接边：预期起始站%1，输入起始站%2").arg(
                           seq.lastVertex()->data.name.toSingleLiteral(),
                           path.front()->from.lock()->data.name.toSingleLiteral()));
        return false;
    }
    double mile=seq.currentMile()+RailNet::pathMile(path);
    appendOperation(PathOperation(path.back()->to.lock(),
                                  PathOperation::AdjRailway,
                                  std::forward<RailNet::path_t>(path),
                                  mile));
    return true;
}

void PathOperationModel::popSelect()
{
    if(seq.empty())return;
    int r=rowCount({});
    beginRemoveRows({},r-1,r-1);
    if(r>1){
        // 非第一行  删除一个结点
        seq.pop();
    }else{
        seq.setStart(nullptr);
    }
    endRemoveRows();
    emit lastVertexChanged(seq.lastVertex());
}

void PathOperationModel::clearSelect()
{
    beginResetModel();
    seq.clear();
    endResetModel();
}

bool PathOperationModel::fromInverseSeq(const PathOperationSeq &other,
                                        QString *report)
{
    if (!other.isValid()){
        report->append(tr("正向径路不能生成径路\n"));
        return false;
    }
    seq.clear();
    addByShortestPath(other.lastVertex(), report);
    for(auto p=other.operations().rbegin();p!=other.operations().rend();++p){
        auto vet=p->path.front()->from.lock();
        if (p->method==PathOperation::ShortestPath){
            bool flag=addByShortestPath(vet, report);
            if(!flag){
                return false;
            }
        }else{
            std::shared_ptr<const RailNet::vertex> stepStart = seq.lastVertex(), stepEnd = vet;
            auto path=net.railPathTo(seq.lastVertex(),vet,
                                     p->railName(),DirFunc::reverse(p->dir()));
            if(path.empty()){
                report->append(tr("按邻线算法查找%1-%2的反向径路失败。\n"
                    "Hint: 出现这种问题，最有可能是选择的路径关键点中存在单向站。"
                    "如无必要，请不要选择单向站作为关键点；或手动给出反向路径。").arg(
                        stepStart->data.name.toSingleLiteral(),
                        stepEnd->data.name.toSingleLiteral()));
                return false;
            }
            bool flag=addByAdjPath(std::move(path),report);
            if(!flag){
                return false;
            }
        }
    }
    return true;
}

void PathOperationModel::addStartStation(
        std::shared_ptr<const RailNet::vertex> station)
{
    beginInsertRows({},0,0);
    seq.setStart(station);
    endInsertRows();
    emit lastVertexChanged(station);
}

void PathOperationModel::appendOperation(PathOperation &&op)
{
    int r=rowCount({});
    beginInsertRows({},r,r);
    seq.push(std::forward<PathOperation>(op));
    endInsertRows();
    emit lastVertexChanged(seq.lastVertex());
}
