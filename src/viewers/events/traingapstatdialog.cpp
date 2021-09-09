#include "traingapstatdialog.h"
#include <QLabel>
#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <utility>
#include <QFormLayout>
#include <QCheckBox>
#include <data/common/qesystem.h>
#include "model/delegate/timeintervaldelegate.h"
#include "data/diagram/diagram.h"
#include "dialogs/trainfilter.h"
#include "util/utilfunc.h"
#include "model/delegate/qedelegate.h"
#include "stationtraingapdialog.h"


TrainGapStatModel::TrainGapStatModel(const TrainGapStatistics &stat_, QObject *parent):
    QStandardItemModel(parent), stat(stat_)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
        tr("位置"),tr("间隔类型"),tr("最小间隔"),tr("数据总量")
        });
    setupModel();
}

void TrainGapStatModel::refreshData()
{
    setupModel();
}

void TrainGapStatModel::setupModel()
{
    using SI=QStandardItem;
    setRowCount(stat.size());
    int row=0;
    for (auto p=stat.begin();p!=stat.end();++p){
        setItem(row,ColPos,new SI(RailStationEvent::posToString(p->first.first)));
        setItem(row,ColType,new SI(TrainGap::typeToString(p->first.second,p->first.first)));
        auto * it=new SI;
        it->setData(p->second.begin()->operator*().secs(),Qt::EditRole);
        setItem(row,ColValue,it);
        it=new SI;
        it->setData(p->second.size(),Qt::EditRole);
        setItem(row,ColCount,it);
        row++;
    }
}


TrainGapStatDialog::TrainGapStatDialog(const TrainGapStatistics &stat, QWidget *parent):
    QDialog(parent),model(new TrainGapStatModel(stat,this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(400,500);
    initUI();
}

void TrainGapStatDialog::refreshData()
{
    model->refreshData();
}

void TrainGapStatDialog::initUI()
{
    auto * vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("这里统计了当前所选计算配置（单双线，车次筛选）条件下，出现的所有"
        "不同类型的列车间隔的最小值以及相应的（该类型出现的）总次数。"
        "在列车间隔分析的窗口中，最小值对应的条目以红字标出"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);
    table=new QTableView();
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->resizeColumnsToContents();
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->setItemDelegateForColumn(TrainGapStatModel::ColValue,
        new TimeIntervalDelegate(this));

    connect(table->horizontalHeader(),
        qOverload<int, Qt::SortOrder>(&QHeaderView::sortIndicatorChanged),
        table,
        qOverload<int, Qt::SortOrder>(&QTableView::sortByColumn));
    table->horizontalHeader()->setSortIndicatorShown(true);

    vlay->addWidget(table);
}

TrainGapSummaryModel::TrainGapSummaryModel(Diagram& diagram_, 
    std::shared_ptr<Railway> railway_, TrainFilter* filter_, QObject* parent):
    QStandardItemModel(parent),diagram(diagram_),railway(railway_),
    events(diagram_.stationEventsForRail(railway_)),
    filter(filter_->getCore())
{
    //refreshData();
}

void TrainGapSummaryModel::refreshData()
{
    setupHeader();
    setupModel();
}

std::shared_ptr<RailStation> TrainGapSummaryModel::stationForRow(int row)
{
    return qvariant_cast<std::shared_ptr<RailStation>>(
        item(row, ColStation)->data(qeutil::RailStationRole));
}


void TrainGapSummaryModel::setupModel()
{
    setRowCount(localMin.size() + 1);
    int row = 0;
    setupRow(row++, tr("全局最小"), globalMin);
    foreach(const auto& st, railway->stations()) {
        if (auto itr = localMin.find(st); itr != localMin.end()) {
            setupRow(row, st->name.toSingleLiteral(), itr->second);
            QVariant v;
            v.setValue(st);
            item(row, ColStation)->setData(v, qeutil::RailStationRole);
            row++;
        }
    }
}

#include <QMap>

void TrainGapSummaryModel::setupHeader()
{
    nextCol = ColOTHERS;
    globalMin.clear();
    localMin.clear();
    typeCols.clear();
    for (auto _p = events.begin(); _p != events.end(); ++_p) {
        const RailStationEventList& lst = _p->second;
        auto gaps = diagram.getTrainGaps(lst, filter, useSingle);
        TrainGapStatistics stat = diagram.countTrainGaps(gaps);
        for (auto q = stat.begin(); q != stat.end(); ++q) {
            const TrainGapTypePair& tp = q->first;
            std::shared_ptr<TrainGap> gap = q->second.begin().operator*();

            // 统计全局最小
            if (auto curmin = globalMin.find(tp); curmin != globalMin.end()) {
                curmin->second = std::min(curmin->second, gap->secs());
            }
            else {
                globalMin.emplace(tp, gap->secs());
            }

            // 登记当前车站的最小
            localMin.operator[](_p->first).emplace(tp, gap->secs());

            // 计算列数
            if (auto itr = typeCols.find(tp); itr == typeCols.end()) {
                typeCols.emplace(tp, nextCol++);
            }
            
        }
    }

    setColumnCount(nextCol);
    QStringList labels({ tr("站名")});
    labels.reserve(nextCol);
    for (int i = ColOTHERS; i < nextCol; i++)
        labels.append("");
    for (auto p = typeCols.begin(); p != typeCols.end(); ++p) {
        labels[p->second] = TrainGap::posTypeToString(p->first.second, p->first.first);
    }
    setHorizontalHeaderLabels(labels);
}

void TrainGapSummaryModel::setupRow(int row, const QString& text, 
    const std::map<TrainGapTypePair, int>& val)
{
    using SI = QStandardItem;
    setItem(row, ColStation, new SI(text));
    for (auto p = val.begin(); p != val.end(); ++p) {
        int c = typeCols.at(p->first);
        auto* it = new SI;
        it->setData(p->second, Qt::EditRole);
        setItem(row, c, it);
    }
}

TrainGapSummaryDialog::TrainGapSummaryDialog(Diagram &diagram,
                                             std::shared_ptr<Railway> railway,
                                             QWidget *parent):
    QDialog(parent),filter(new TrainFilter(diagram,this)),
    model(new TrainGapSummaryModel(diagram,railway,filter,this)),
    dele(new TimeIntervalDelegate(this))
{
    setWindowTitle(tr("列车间隔汇总 - %1").arg(railway->name()));
    setAttribute(Qt::WA_DeleteOnClose);
    resize(800,800);
    initUI();

    refreshData();
}

void TrainGapSummaryDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);

    auto* flay=new QFormLayout;
    auto* hlay=new QHBoxLayout;
    ckSingle=new QCheckBox(tr("对向列车敌对进路 (单线模式)"));
    hlay->addWidget(ckSingle);
    auto* btn=new QPushButton(tr("车次筛选器"));
    connect(btn,&QPushButton::clicked,filter,
            &TrainFilter::show);
    hlay->addWidget(btn);
    hlay->addStretch(1);
    btn=new QPushButton(tr("刷新"));
    connect(btn,&QPushButton::clicked,this,&TrainGapSummaryDialog::refreshData);
    hlay->addWidget(btn);
    flay->addRow(tr("选项"),hlay);
    vlay->addLayout(flay);

    auto* lab=new QLabel(tr("此功能一次性统计所有车站的所有列车间隔情况。" 
        "下表列出的数值是符合条件的最小间隔时长。第一行为所有车站的最小值汇总。\n" 
        "可以在上方更改配置、筛选参与计算的车次，修改后请点击[刷新]来显示最新结果。" 
        "对于较为复杂的运行图文件，可能耗费较多计算时间。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->setModel(model);
    vlay->addWidget(table);

    connect(table, &QTableView::doubleClicked, this,
        &TrainGapSummaryDialog::onDoubleClicked);

    auto* g = new ButtonGroup<2>({ "导出CSV","关闭" });
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()), this, { SLOT(toCsv()),SLOT(close()) });
}



void TrainGapSummaryDialog::onDoubleClicked(const QModelIndex& idx)
{
    if (!idx.isValid() || idx.row() == 0)return;
    auto st = model->stationForRow(idx.row());
    decltype(auto) events = model->getEvents();
    auto* dlg = new StationTrainGapDialog(model->getDiagram(), model->getRailway(),
        st, events.at(st), filter, this, ckSingle->isChecked());
    dlg->show();
}

void TrainGapSummaryDialog::toCsv()
{
    QString text = QString("%1_间隔汇总").arg(model->getRailway()->name());
    qeutil::exportTableToCsv(model, this, text);
}

void TrainGapSummaryDialog::refreshData()
{
    model->setUseSingle(ckSingle->isChecked());
    model->refreshData();
    
    for (int i = TrainGapSummaryModel::ColOTHERS; i < model->columnCount(); i++) {
        table->setItemDelegateForColumn(i, dele);
    }
    table->resizeColumnsToContents();
}
