#include "railsnapevents.h"

#include "data/rail/railstation.h"
#include "data/rail/railway.h"
#include "util/buttongroup.hpp"
#include "util/utilfunc.h"
#include "data/train/routing.h"
#include "model/delegate/generaldoublespindelegate.h"
#include "util/pagecomboforrail.h"
#include "data/common/qesystem.h"
#include "data/train/train.h"
#include "data/train/traintype.h"
#include "data/diagram/diagram.h"
#include "util/traintimeedit.h"

#include <QLabel>
#include <QTableView>
#include <QHeaderView>
#include <QAction>
#include <QScroller>

RailSnapEventsModel::RailSnapEventsModel(Diagram &diagram_,
                                         std::shared_ptr<Railway> railway_,
                                         QObject *parent):
    QStandardItemModel(parent),diagram(diagram_),railway(railway_)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
      tr("车次"),tr("里程标"),tr("位置"),tr("类型"),tr("状态"),
      tr("方向"),tr("始发"),tr("终到"),tr("车底"),tr("担当"),tr("备注")
                              });
}

void RailSnapEventsModel::setTime(const TrainTime &time)
{
    this->time=time;
    lst=diagram.getSnapEvents(railway,time);
    setupModel();
}

double RailSnapEventsModel::mileForRow(int row) const
{
    return item(row, ColMile)->data(Qt::EditRole).toDouble();
}

void RailSnapEventsModel::setupModel()
{
    beginResetModel();
    setRowCount(lst.size());
    using SI=QStandardItem;

    for(int i=0;i<lst.size();i++){
        const auto& e=lst.at(i);
        auto train=e.line->train();
        setItem(i,ColTrainName,new SI(train->trainName().full()));
        auto* it = new SI;
        it->setData(e.mile, Qt::EditRole);
        setItem(i, ColMile, it);
        if(e.isStationEvent()){
            auto st=std::get<std::shared_ptr<const RailStation>>(e.pos);
            setItem(i,ColPos,new SI(st->name.toSingleLiteral()));
        }else{
            auto itv=std::get<std::shared_ptr<const RailInterval>>(e.pos);
            setItem(i,ColPos,new SI(itv->toString()));
        }
        setItem(i,ColType,new SI(train->type()->name()));
        setItem(i,ColStatus,new SI(
                  e.isStopped?tr("停车"):tr("运行")));
        setItem(i,ColDir,new SI(DirFunc::dirToString(e.line->dir())));
        setItem(i,ColStarting,new SI(train->starting().toSingleLiteral()));
        setItem(i,ColTerminal,new SI(train->terminal().toSingleLiteral()));
        if(train->hasRouting()){
            auto rt=train->routing().lock();
            setItem(i,ColModel,new SI(rt->model()));
            setItem(i,ColOwner,new SI(rt->owner()));
        }else{
            setItem(i,ColModel,new SI("-"));
            setItem(i,ColOwner,new SI("-"));
        }
        setItem(i,ColNote,new SI(e.note));
    }

    endResetModel();
}



RailSnapEventsDialog::RailSnapEventsDialog(Diagram &diagram_,
                      std::shared_ptr<Railway> railway_, QWidget *parent):
    QDialog(parent),diagram(diagram_),railway(railway_),
    model(new RailSnapEventsModel(diagram_,railway_,this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("线路快照 - %1").arg(railway->name()));
    resize(800,800);
    initUI();
}

void RailSnapEventsDialog::initUI()
{
    auto* vlay=new QVBoxLayout;
    auto* hlay=new QHBoxLayout;
    hlay->addWidget(new QLabel("时刻"));
    timeEdit=new TrainTimeEdit;
    timeEdit->setMaxHours(diagram.options().period_hours);
    timeEdit->setFormat(TrainTime::HMS);
    hlay->addWidget(timeEdit);
    hlay->addStretch(1);
    auto* btn=new QPushButton(tr("确定"));
    connect(btn,SIGNAL(clicked()),this,SLOT(updateData()));
    hlay->addWidget(btn);
    vlay->addLayout(hlay);

    table=new QTableView;
    table->setModel(model);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    table->horizontalHeader()->setSortIndicatorShown(true);
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->setItemDelegateForColumn(RailSnapEventsModel::ColMile,
        new GeneralDoubleSpinDelegate(this));
    auto* act = new QAction(tr("定位到运行图"), this);
    table->addAction(act);
    connect(act, &QAction::triggered, this, &RailSnapEventsDialog::actLocate);
    connect(table->horizontalHeader(),SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            table,SLOT(sortByColumn(int,Qt::SortOrder)));
    table->setContextMenuPolicy(Qt::ActionsContextMenu);
    QScroller::grabGesture(table,QScroller::TouchGesture);
    vlay->addWidget(table);
    auto* g=new ButtonGroup<2>({"导出CSV","关闭"});
    g->connectAll(SIGNAL(clicked()),this,{SLOT(toCsv()),SLOT(close())});
    vlay->addLayout(g);
    setLayout(vlay);
}

void RailSnapEventsDialog::updateData()
{
    const TrainTime& tm=timeEdit->time();
    model->setTime(tm);
    table->resizeColumnsToContents();
}

void RailSnapEventsDialog::toCsv()
{
    QString s=tr("%1运行快照").arg(railway->name());
    qeutil::exportTableToCsv(model, table, this, s);
}

void RailSnapEventsDialog::actLocate()
{
    auto&& idx = table->currentIndex();
    if (!idx.isValid())return;
    int pageIndex = PageComboForRail::dlgGetPageIndex(diagram, railway, this,
        tr("选择运行图"), tr("请选择要定位到的运行图页面："));
    if (pageIndex < 0)return;
    emit locateToEvent(pageIndex, railway, model->mileForRow(idx.row()), timeEdit->time());
}
