#include "trainlinenet.h"
#include "data/train/train.h"
#include "data/diagram/trainline.h"
#include "data/diagram/trainadapter.h"
#include "data/rail/railway.h"
#include "railnet/graph/railnetutil.hpp"

void TrainLineNet::fromBoundTrain(const Train &train)
{
    foreach(auto adp, train.adapters()){
        foreach(auto line, adp->lines()){
            addTrainLine(*adp->railway(),*line);
        }
    }
}

TrainLineNet::TrainLinePathResult TrainLineNet::pathBetween(
        const TrainStation *from, const TrainStation *to)
{
    TrainLinePathResult res;
    auto v_from=find_vertex(from);
    auto v_to=find_vertex(to);
    if (!v_from || !v_to) {
        res.exists = false;
        res.path_s = QObject::tr("起始站或终止站未铺画");
        return res;
    }
    auto sssp_res=sssp(v_from,&GraphInterval::getMile);
    auto path=dump_path(v_from,v_to,sssp_res);
    if (path.empty()){
        // not reachable
        res.exists=false;
        res.path_s=QObject::tr("起始站至终止站之间的列车运行线无法构成完整路径");
    }else{
        res.exists=true;
        res.mile=sssp_res.distance.at(v_to);
        res.path_s=railnetutil::pathToString<const TrainStation*>(path);
    }
    return res;
}

void TrainLineNet::addTrainLine(const Railway &rail, const TrainLine &line)
{
    if (line.isNull()) return;
    auto start=line.firstRailStation(), end=line.lastRailStation();

    // now loop all the intervals in the TrainLine (!!)
    auto pre=emplace_vertex(&*line.firstStation()->trainStation, *line.firstRailStation());

    // loop invariant: each time the interval ENDED by the p is processed
    auto p=std::next(line.stations().begin());   // it is safe for non-empty line
    for(;p!=line.stations().end();++p){
        auto post=emplace_vertex(&*(*p).trainStation, *(*p).railStation.lock());
        double mile=line.previousBoundIntervalMile(p);
        if (mile>=0) [[likely]]{
            emplace_edge(pre,post,rail.name(), line.dir(),mile);
        }else [[unlikely]]{
            qDebug()<<"TrainLineNet::addTrainLine: WARNING: negative mile encountered "
                   << mile<<Qt::endl;
        }
        pre=std::move(post);
    }

}
