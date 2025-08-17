#include "routingmiledialog.h"

#include "data/train/routing.h"
#include "data/train/train.h"
#include "util/utilfunc.h"
#include "data/analysis/runstat/trainintervalstat.h"
#include "data/common/qesystem.h"

#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>


RoutingMileModel::RoutingMileModel(const DiagramOptions& ops, std::shared_ptr<Routing> routing, QObject *parent):
    QStandardItemModel(parent), m_ops(ops), m_routing(routing)
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

            TrainIntervalStat stat{m_ops, itr->train()};
            stat.setRange(0, itr->train()->stationCount() - 1);
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

        it = new SI;
        it->setEditable(false);
        it->setCheckState(qeutil::boolToCheckState(itr->link()));
        it->setCheckable(false);
        setItem(r, ColLink, it);
    }
}

RoutingMileDialog::RoutingMileDialog(const DiagramOptions& ops, std::shared_ptr<Routing> routing, QWidget *parent):
    QDialog(parent), m_ops(ops), m_routing(routing), m_model(new RoutingMileModel(m_ops, routing, this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("交路里程 - %1").arg(m_routing->name()));
    initUI();
}

void RoutingMileDialog::initUI()
{
    resize(800, 600);
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("本窗口提供交路中各次列车的总里程，以及交路累计里程的统计。"
                              "注意，由于列车并不一定全程在线路上有铺画运行线，其全程里程并不总是可得，"
                              "交路里程也并不总是可得。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    m_table=new QTableView;
    m_table->setModel(m_model);

    m_table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    m_table->setEditTriggers(QTableView::NoEditTriggers);

    {
        int c = 0;
        for(int w: {120, 40, 100, 100, 40, 100, 100}) {
            m_table->setColumnWidth(c++, w);
        }
    }

    vlay->addWidget(m_table);
}
