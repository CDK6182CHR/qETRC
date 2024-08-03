#include "routingmiledialog.h"

#include "data/train/routing.h"
#include "data/train/train.h"
#include "util/utilfunc.h"
#include "data/analysis/runstat/trainintervalstat.h"


RoutingMileModel::RoutingMileModel(std::shared_ptr<Routing> routing, QObject *parent):
    QStandardItemModel(parent), m_routing(routing)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
                               tr("车次"), tr("虚拟"),tr("始发"),tr("终到"),
        tr("连线"),tr("里程"),tr("累计里程")
    });
    setupModel();
}

void RoutingMileModel::setupModel()
{
    using SI=QStandardItem;
    setRowCount(m_routing->count());

    int r=0;
    bool mile_valid=true;
    double acc_mile=0;
    for(auto itr=m_routing->order().begin(); itr!=m_routing->order().end();++itr,++r) {
        setItem(r, ColTrainName, new SI(itr->name()));

        auto* it=qeutil::makeCheckItem();
        it->setCheckState(qeutil::boolToCheckState(itr->isVirtual()));
        it->setCheckable(false);
        setItem(r,ColVirtual,it);

        if (itr->isVirtual()) {
            mile_valid=false;
            setItem(r,ColStarting,new SI(itr->virtualStarting()));
            setItem(r,ColTerminal,new SI(itr->virtualTerminal()));
            qInfo() << "Cannot compute mile for virtual train " << itr->name();
        }else {
            setItem(r,ColStarting,new SI(itr->train()->starting().toSingleLiteral()));
            setItem(r,ColTerminal,new SI(itr->train()->terminal().toSingleLiteral()));

            TrainIntervalStat stat{itr->train()};
            auto res = stat.compute();
            if (res.railResults.isValid) {
                double this_mile = res.railResults.totalMiles;
                setItem(r, ColMile, new SI(QString::number(this_mile, 'f', 3)));

                if (mile_valid) {
                    acc_mile += this_mile;
                    setItem(r, ColAccMile, new SI(QString::number(acc_mile, 'f', 3)));
                }
            }else {
                // data invalid
                qInfo() << "Cannot compute total mile for train " << itr->name();
                mile_valid=false;
            }
        }
    }
}
