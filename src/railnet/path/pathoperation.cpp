#include "pathoperation.h"


QString PathOperation::railName() const
{
    if (path.empty()){
        qDebug()<<"PathOperation::railName: WARNING: empty path"<<Qt::endl;
        return {};
    }else{
        return path.front()->data.railName;
    }
}

Direction PathOperation::dir() const
{
    if(path.empty()){
        qDebug()<<"PathOperation::dir: WARNING: empty path"<<Qt::endl;
        return {};
    }else{
        return path.front()->data.dir;
    }
}

QString PathOperation::methodString(Method m)
{
    switch (m){
    case ShortestPath: return QObject::tr("最短路");
    case AdjStation: return QObject::tr("邻站");
    case AdjRailway: return QObject::tr("邻线");
    default: return "INVALID METHOD";
    }
}

void PathOperationSeq::push(PathOperation &&op)
{
    _operations.push_back(std::forward<PathOperation>(op));
}

std::shared_ptr<const RailNet::vertex> PathOperationSeq::lastVertex() const
{
    if (_operations.empty())
        return _start;
    else return _operations.back().target;
}

bool PathOperationSeq::isValid() const
{
    return _start && ! _operations.empty();
}

void PathOperationSeq::clear()
{
    _start.reset();
    _operations.clear();
}

bool PathOperationSeq::empty() const
{
    return !_start && _operations.empty();
}

double PathOperationSeq::currentMile() const
{
    if (_operations.empty())
        return 0;
    else return _operations.back().mileStone;
}

RailNet::path_t PathOperationSeq::fullPath() const
{
    RailNet::path_t res;
    for(const auto& op:_operations){
        res.insert(res.end(),op.path.begin(),op.path.end());
    }
    return res;
}
