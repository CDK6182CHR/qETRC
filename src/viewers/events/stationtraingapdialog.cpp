#include "stationtraingapdialog.h"
#include "data/diagram/traingap.h"
#include "data/diagram/trainline.h"
#include "data/diagram/diagram.h"
#include "model/delegate/qedelegate.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableView>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QAction>
#include <QScroller>
#include "util/buttongroup.hpp"
#include <data/common/qesystem.h>
#include "util/utilfunc.h"
#include "model/delegate/timeintervaldelegate.h"
#include "traingapstatdialog.h"
#include "dialogs/trainfilter.h"
#include "util/pagecomboforrail.h"


StationTrainGapModel::StationTrainGapModel(Diagram& diagram, 
    std::shared_ptr<Railway> railway_,
    std::shared_ptr<RailStation> station_,
    const RailStationEventList& events, 
    const TrainFilterCore& filter, QObject* parent, bool useSingleLine,int cutSecs_):
    QStandardItemModel(parent),diagram(diagram), railway(railway_),
    station(station_),events(events),
    filter(filter),singleLine(useSingleLine),_cutSecs(cutSecs_)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
        tr("间隔类型"),tr("间隔时长"),tr("位置"),
        tr("前车次"),tr("前时刻"), tr("后车次"),tr("后时刻"),tr("数量")
        });
}

void StationTrainGapModel::refreshData()
{
    data = diagram.getTrainGaps(events, filter, singleLine);
    stat = diagram.countTrainGaps(data, _cutSecs);
    setupModel();
}

std::shared_ptr<TrainGap> StationTrainGapModel::gapForRow(int row)
{
    return qvariant_cast<std::shared_ptr<TrainGap>>(
        item(row, ColGapType)->data(qeutil::TrainGapRole));
}

void StationTrainGapModel::setupModel()
{
    using SI = QStandardItem;
    setRowCount(data.size());
    for (int i = 0; i < data.size(); i++) {
        const auto& ev = *(data.at(i));
        auto* it = new SI(ev.typeString());
        QVariant v;
        v.setValue(data.at(i));
        it->setData(v, qeutil::TrainGapRole);
        setItem(i, ColGapType, it);
        it = new SI;
        it->setData(ev.secs(), Qt::EditRole);
        setItem(i, ColPeriod, it);
        setItem(i, ColPos, new SI(RailStationEvent::posToString(ev.position())));
        setItem(i, ColLeftTrain, new SI(ev.left->line->train()->trainName().full()));
        setItem(i, ColLeftTime, new SI(ev.left->time.toString("hh:mm:ss")));
        setItem(i, ColRightTrain, new SI(ev.right->line->train()->trainName().full()));
        setItem(i, ColRightTime, new SI(ev.right->time.toString("hh:mm:ss")));
        it = new SI;
        it->setData(ev.num, Qt::EditRole);
        setItem(i, ColNumber, it);
        QColor color(Qt::black);
        if (isMinimumGap(ev)) {
            color = Qt::red;
        }
        setRowColor(i, color);
    }
}

bool StationTrainGapModel::isMinimumGap(const TrainGap& gap) const
{
    if (gap.position() == RailStationEvent::NoPos) {
        // 注意由于存在截断秒数，这样去查找可能查出来是空的
        // 但找出来的，value一定不是空的
        if (auto p = stat.find(std::make_pair(gap.position(), gap.type)); p != stat.end()) {
            return gap.secs() == p->second.begin()->operator*().secs();
        }
        else return false;
    }
    // 不是NoPos的：按两个边界分别找
    if (gap.position() & RailStationEvent::Pre){
        if (auto p = stat.find(std::make_pair(RailStationEvent::Pre, gap.type)); p != stat.end()) {
            if (p->second.begin()->operator*().secs() == gap.secs())
                return true;
        }
    }
    if (gap.position() & RailStationEvent::Post) {
        if (auto p = stat.find(std::make_pair(RailStationEvent::Post, gap.type)); p != stat.end()) {
            if (p->second.begin()->operator*().secs() == gap.secs())
                return true;
        }
    }
    return false;
}

void StationTrainGapModel::setRowColor(int row, const QColor& color)
{
    for (int c = 0; c < ColMAX; c++) {
        item(row, c)->setForeground(color);
    }
}


StationTrainGapDialog::StationTrainGapDialog(Diagram& diagram, 
    std::shared_ptr<Railway> railway_, 
    std::shared_ptr<RailStation> station_, 
    const RailStationEventList& events, 
    TrainFilter* filter_, QWidget* parent,bool useSingleLine,
    int cutSecs):
    QDialog(parent), railway(railway_), station(station_),
    model(new StationTrainGapModel(diagram,railway_,station_,events,
        filter_->getCore(),this,useSingleLine,cutSecs)),
    filter(filter_)
{
    setWindowTitle(tr("车站间隔分析 - %1 @ %2").arg(station->name.toSingleLiteral(),
        railway->name()));
    setAttribute(Qt::WA_DeleteOnClose);
    resize(700, 700);
    initUI();
    refreshData();
}

StationTrainGapDialog::StationTrainGapDialog(Diagram& diagram, 
    std::shared_ptr<Railway> railway_, 
    std::shared_ptr<RailStation> station_, QWidget* parent):
    QDialog(parent),railway(railway_),station(station_),
    filter(new TrainFilter(diagram,this)),
    model(new StationTrainGapModel(diagram,railway_,station_,
        diagram.stationEvents(railway_,station_),filter->getCore(),this))
{
    setWindowTitle(tr("车站间隔分析 - %1 @ %2").arg(station->name.toSingleLiteral(),
        railway->name()));
    setAttribute(Qt::WA_DeleteOnClose);
    resize(700, 700);
    initUI();
    refreshData();
}


void StationTrainGapDialog::refreshData()
{
    model->setSingleLine(ckSingle->isChecked());
    model->setCutSecs(spCut->value());
    model->refreshData();
    table->resizeColumnsToContents();
}


void StationTrainGapDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;
    auto* hlay=new QHBoxLayout;
    ckSingle=new QCheckBox(tr("对向敌对进路 (单线模式)"));
    ckSingle->setChecked(model->isSingleLine());
    hlay->addWidget(ckSingle);
    auto* btn=new QPushButton(tr("车次筛选器"));
    hlay->addWidget(btn);
    connect(btn,&QPushButton::clicked,filter,&TrainFilter::show);
    hlay->addStretch(1);
    btn=new QPushButton(tr("刷新"));
    hlay->addWidget(btn);
    connect(btn,&QPushButton::clicked,this,&StationTrainGapDialog::refreshData);
    flay->addRow(tr("选项"),hlay);
    vlay->addLayout(flay);

    spCut = new QSpinBox;
    spCut->setMaximumWidth(200);
    spCut->setRange(0, 10000000);
    spCut->setSingleStep(10);
    spCut->setToolTip(tr("截断秒数\n小于所给秒数的间隔不参与最小值统计。"
        "注意它们还是会出现在间隔表中，只是计算最小值时不予考虑。"));
    spCut->setSuffix(tr(" 秒 (s)"));
    flay->addRow(tr("最小间隔"), spCut);
    spCut->setValue(model->cutSecs());

    auto* lab=new QLabel(tr("下表给出所有相邻事件之间的时间间隔情况。"
        "可选择单线或双线模式（区别在于是否考虑对向车次之间的敌对进路）。\n"
        "只有发生在车站同一端（站前或站后，即里程小端或大端）的事件才被认为是相关的，其间存在间隔"
        "（例如，同一车站的下行终到与下行始发事件不相关，不存在间隔）。\n"
        "标红的条目为当前类目中的最小间隔。可点击[统计]查看所有类目的最小间隔。"
        ""));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->horizontalHeader()->setSortIndicatorShown(true);
    table->setItemDelegateForColumn(StationTrainGapModel::ColPeriod,
        new TimeIntervalDelegate(this));
    connect(table->horizontalHeader(),
            qOverload<int,Qt::SortOrder>(&QHeaderView::sortIndicatorChanged),
            table,
            qOverload<int,Qt::SortOrder>(&QTableView::sortByColumn)
            );
    vlay->addWidget(table);
    QScroller::grabGesture(table,QScroller::TouchGesture);

    auto* act = new QAction(tr("定位到前事件"), table);
    connect(act, &QAction::triggered, this, &StationTrainGapDialog::locateLeft);
    table->addAction(act);
    act = new QAction(tr("定位到后事件"), table);
    connect(act, &QAction::triggered, this, &StationTrainGapDialog::locateRight);
    table->addAction(act);
    table->setContextMenuPolicy(Qt::ActionsContextMenu);

    auto* g=new ButtonGroup<3>({"导出CSV","统计","关闭"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actToCsv()),SLOT(actStatistics()),
        SLOT(close())});
}


void StationTrainGapDialog::actStatistics()
{
    auto* dialog = new TrainGapStatDialog(model->getStat(), this);
    dialog->setWindowTitle(tr("车站间隔统计 - %1 @ %2").arg(station->name.toSingleLiteral(),
        railway->name()));
    dialog->show();
}

void StationTrainGapDialog::locateLeft()
{
    auto&& idx = table->currentIndex();
    if (!idx.isValid())return;
    auto gap = model->gapForRow(idx.row());
    int pageIndex = PageComboForRail::dlgGetPageIndex(model->getDiagram(), railway, this,
        tr("定位到事件"), tr("请选择要定位到的运行图页面"));
    if (pageIndex < 0)return;
    emit locateToEvent(pageIndex, railway, gap->left->station.lock(), gap->left->time);
}

void StationTrainGapDialog::locateRight()
{
    auto&& idx = table->currentIndex();
    if (!idx.isValid())return;
    auto gap = model->gapForRow(idx.row());
    int pageIndex = PageComboForRail::dlgGetPageIndex(model->getDiagram(), railway, this,
        tr("定位到事件"), tr("请选择要定位到的运行图页面"));
    if (pageIndex < 0)return;
    emit locateToEvent(pageIndex, railway, gap->right->station.lock(), gap->right->time);
}

void StationTrainGapDialog::actToCsv()
{
    QString text=QString("%1_%2间隔分析").arg(railway->name(),station->name.toSingleLiteral());
    qeutil::exportTableToCsv(model,this,text);
}
