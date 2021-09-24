#include "railnet.h"
#include "data/rail/railway.h"
#include "data/rail/railcategory.h"
#include "data/rail/forbid.h"


void RailNet::fromRailCategory(const RailCategory *cat)
{
    foreach(const auto& sub, cat->subCategories()){
        fromRailCategory(sub.get());
    }
    foreach(const auto& rail, cat->railways()){
        addRailway(rail.get());
    }
}

std::shared_ptr<Railway> RailNet::sliceByPath(QVector<QString> points,
                                              QString *report)
{
    auto rdown=singleRailFromPath(points, report);
    if(!rdown){
        report->append(QObject::tr("寻找正向径路错误: "));
        return rdown;
    }

    std::reverse(points.begin(), points.end());

    auto rup=singleRailFromPath(points, report);
    if(!rup){
        report->append(QObject::tr("寻找反向径路错误："));
        return rup;
    }
    rdown->mergeCounter(*rup);
    return rdown;
}

std::shared_ptr<Railway> RailNet::singleRailFromPath(const QVector<QString> &points,
                                                     QString *report)
{
    if(points.size()<=1){
        report->append(QObject::tr("至少需给出两个关键点"));
        return nullptr;
    }
    auto res=std::make_shared<Railway>(
                QString("%1-%2").arg(points.front(),points.back()));
    res->ensureForbids(Forbid::FORBID_COUNT);

    QMap<QString,int> rulerMap;

    auto prev=points.begin();
    auto prst=find_vertex(*prev);
    if(!prst){
        report->append(QObject::tr("径路首站%1不在图中").arg(*prev));
        return nullptr;
    }
    res->appendStation(prst->data.name,0,prst->data.level,std::nullopt,
                       PassedDirection::DownVia,true,prst->data.passenger,
                       prst->data.freight);

    double mile=0;
    for(auto p=std::next(prev);p!=points.end();prev=p,++p){
        auto curst=find_vertex(*p);
        if(!curst){
            report->append(QObject::tr("径路中间站%1不在图中").arg(*p));
            return nullptr;
        }
        auto ret=sssp(prst, &GraphInterval::getMile);
        auto subpath=dump_path(prst,curst,ret);

        if(subpath.empty()){
            report->append(QObject::tr("区间[%1->%2]不可达").arg(
                               *prev,*p));
            return nullptr;
        }

        for(const auto& e: subpath){
            const auto& d=e->to.lock()->data;
            mile+=e->data.mile;
            res->appendStation(d.name,mile,d.level,std::nullopt,
                               PassedDirection::DownVia,true,
                               d.passenger,d.freight);
            auto railint=res->lastDownInterval();
            const auto& dint=e->data;
            // 天窗
            for(int i=0;i<std::min(res->forbids().size(),
                                   dint.forbidNodes.size());i++){
                auto node = railint->getForbidNode(res->forbids().at(i));
                dint.forbidNodes.at(i).exportToNode(*node);
            }

            // 标尺
            foreach(const auto& rn, dint.rulerNodes){
                int idx;
                if (auto itr=rulerMap.find(rn.name);itr!=rulerMap.end()){
                    idx=itr.value();
                }else{
                    idx=rulerMap.size();
                    rulerMap.insert(rn.name,idx);
                    res->addEmptyRuler(rn.name,true);
                }
                auto ruler=res->getRuler(idx);
                auto node=railint->getRulerNode(ruler);
                rn.exportToNode(*node);
            }
        }

        prst=curst;
    }
    return res;

}

void RailNet::addRailway(const Railway *railway)
{
    std::shared_ptr<vertex> pre{};
    for(auto p=railway->firstDownInterval();p;p=railway->nextIntervalCirc(p)){
        if (!pre){
            pre=emplace_vertex(p->fromStation()->name, *(p->fromStation()));
        }
        auto post=emplace_vertex(p->toStation()->name, *(p->toStation()));
        auto ed=emplace_edge(pre,post,railway->name(),*p);
        pre=std::move(post);
        if (p->mile() < 0) {
            qDebug() << "RailNet::addRailway: ERROR: negative mile, set to 0 " << p->mile() <<
                *p << " @ " << railway->name() << Qt::endl;
            ed->data.mile = 0;
        }
    }
}
