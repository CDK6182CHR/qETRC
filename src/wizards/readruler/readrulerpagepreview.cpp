#include "readrulerpagepreview.h"

#ifndef QETRC_MOBILE_2

#include "data/common/qesystem.h"
#include "util/utilfunc.h"
#include "util/dialogadapter.h"
#include "data/train/train.h"
#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QAction>

ReadRulerPreviewModel::ReadRulerPreviewModel(const ReadRulerReport& data_,
    const QVector<std::shared_ptr<RailInterval>>& intervals_, QObject* parent):
	QStandardItemModel(parent),data(data_),intervals(intervals_)
{
	setColumnCount(ColMAX);
	setHorizontalHeaderLabels({
		tr("区间"),tr("通通时分"),tr("起步附加"),tr("停车附加"),tr("数据类"),tr("数据总量"),
		tr("满足车次")
		});
}

void ReadRulerPreviewModel::refreshData()
{
    using SI = QStandardItem;
    beginResetModel();
    setRowCount(intervals.size());
    int row = 0;
    foreach(auto it, intervals) {
        auto&& rep = data.at(it);
        //数据类和数据总量可以直接往里填
        setItem(row, ColInterval, new SI(it->toString()));
        setItem(row, ColDataCount, new SI(QString::number(rep.raw.size())));
        setItem(row, ColTypeCount, new SI(QString::number(rep.types.size())));
        if (rep.isValid()) {
            //区间标尺有效
            setItem(row, ColPass, new SI(qeutil::secsDiffToString(rep.interval)));
            setItem(row, ColStart, new SI(qeutil::secsDiffToString(rep.start)));
            setItem(row, ColStop, new SI(qeutil::secsDiffToString(rep.stop)));
            setItem(row, ColSatisfied, new SI(QString::number(rep.satisfiedCount())));
        }
        else {
            setItem(row, ColPass, new SI("-"));
            setItem(row, ColStart, new SI("-"));
            setItem(row, ColStop, new SI("-"));
            setItem(row, ColSatisfied, new SI("-"));
        }
        row++;
    }
    endResetModel();
}

ReadRulerPagePreview::ReadRulerPagePreview(QWidget* parent):
	QWizardPage(parent),model(new ReadRulerPreviewModel(data,intervals, this))
{
	initUI();
}

void ReadRulerPagePreview::setData(ReadRulerReport&& _report, 
    const QVector<std::shared_ptr<RailInterval>>& _intervals, bool useAverage)
{
    this->data = std::forward<ReadRulerReport>(_report);
    this->intervals = _intervals;
    this->useAverage = useAverage;

    model->refreshData();
    table->resizeColumnsToContents();
}

void ReadRulerPagePreview::initUI()
{
    setTitle(tr("预览"));
    setSubTitle("计算结果如下表所示。\n"
                           "点击[完成]应用结果，否则结果不会被应用。\n"
                           "双击行显示详细计算数据。\n"
                           "表中的[数据类]是指在[通通]/[起通]/[通停]/[起停]这四种情况中，"
                           "有多少种情况的有效数据；"
                           "[数据总量]是指用于计算的数据总条数；[满足车次]是指用于计算的车次中，"
                           "严格满足标尺的数量。"
                           );
    auto* vlay=new QVBoxLayout(this);
    table=new QTableView;
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    vlay->addWidget(table);

    // actions ..
    auto* act = new QAction(tr("显示各车次数据 （双击）"),table);
    connect(act, SIGNAL(triggered()), this, SLOT(actShowDetail()));
    table->addAction(act);
    act = new QAction(tr("按类型整理数据"), table);
    connect(act, SIGNAL(triggered()), this, SLOT(actShowSummary()));
    table->addAction(act);
    table->setContextMenuPolicy(Qt::ActionsContextMenu);
    connect(table, &QTableView::doubleClicked, this,
        &ReadRulerPagePreview::onDoubleClicked);

    // detail
    mdDetail = new ReadRulerDetailModel(this);
    tbDetail = new QTableView;
    tbDetail->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    tbDetail->setEditTriggers(QTableView::NoEditTriggers);
    tbDetail->setModel(mdDetail);
    dlgDetail = new DialogAdapter(tbDetail, this);
    dlgDetail->setAttribute(Qt::WA_DeleteOnClose, false);
    tbDetail->horizontalHeader()->setSortIndicatorShown(true);
    connect(tbDetail->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
        tbDetail, SLOT(sortByColumn(int, Qt::SortOrder)));

    //summary
    mdSummary = new ReadRulerSummaryModel(this);
    tbSummary = new QTableView;
    tbSummary->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    tbSummary->setEditTriggers(QTableView::NoEditTriggers);
    tbSummary->setModel(mdSummary);
    dlgSummary = new DialogAdapter(tbSummary, this);
    dlgSummary->setAttribute(Qt::WA_DeleteOnClose, false);
    tbSummary->horizontalHeader()->setSortIndicatorShown(true);
    connect(tbSummary->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
        tbSummary, SLOT(sortByColumn(int, Qt::SortOrder)));

}

void ReadRulerPagePreview::actShowDetail()
{
    onDoubleClicked(table->selectionModel()->currentIndex());
}

void ReadRulerPagePreview::actShowSummary()
{
    const auto& idx = table->selectionModel()->currentIndex();
    if (!idx.isValid())
        return;
    auto it = intervals.at(idx.row());
    dlgSummary->setWindowTitle(tr("类型数据 - [%1]").arg(it->toString()));
    mdSummary->setupModel(data.at(it));
    tbSummary->resizeColumnsToContents();
    if (!dlgSummary->isVisible())
        dlgSummary->show();
    else
        dlgSummary->activateWindow();
}

void ReadRulerPagePreview::onDoubleClicked(const QModelIndex& idx)
{
    if (!idx.isValid())return;
    auto it = intervals.at(idx.row());
    dlgDetail->setWindowTitle(tr("计算细节 - [%1]").arg(it->toString()));
    mdDetail->setupModel(it, data.at(it), useAverage);
    tbDetail->resizeColumnsToContents();
    if (!dlgDetail->isVisible())
        dlgDetail->show();
    else
        dlgDetail->activateWindow();
}

ReadRulerDetailModel::ReadRulerDetailModel(QObject* parent):
    QStandardItemModel(parent)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
        tr("车次"),tr("附加"),tr("标准"),tr("实际"),tr("差时"),tr("绝对值"),tr("标记")
        });
}

void ReadRulerDetailModel::setupModel(std::shared_ptr<RailInterval> railint, 
    const readruler::IntervalReport& itrep, bool useAverage)
{
    Q_UNUSED(railint);
    using SI = QStandardItem;
    setRowCount(static_cast<int>(itrep.raw.size()));
    int row = 0;
    for (auto p = itrep.raw.begin(); p != itrep.raw.end(); ++p) {
        TrainLine::IntervalAttachType tp = p->second.second;
        setItem(row, ColTrainName, new SI(p->first->trainName().full()));
        setItem(row, ColAttach, new SI(TrainLine::attachTypeString(p->second.second)));
        int secstd = itrep.stdInterval(p->second.second);
        setItem(row, ColStd, new SI(qeutil::secsDiffToString(secstd)));
        int secreal = p->second.first;
        setItem(row, ColReal, new SI(qeutil::secsDiffToString(secreal)));
        int ds = secreal - secstd;
        setItem(row, ColDiff, new SI(QString::number(ds)));
        setItem(row, ColDiffAbs, new SI(QString::number(std::abs(ds))));

        if (useAverage) {
            // 均值模式，标记截断的数据
            auto& tpcnt = itrep.types.at(tp).count;
            if (auto p = tpcnt.find(secreal); p == tpcnt.end()) {
                setItem(row, ColMark, new SI(tr("截断")));
            }
        }
        else {
            // 众数模式，标记采信数据和类型弃用数据
            if (!itrep.types.at(tp).used) {
                setItem(row, ColMark, new SI(tr("类型弃用")));
            }
            else if (secreal == itrep.types.at(tp).value) {
                setItem(row, ColMark, new SI(tr("采信")));
            }
        }
        row++;
    }
}

ReadRulerSummaryModel::ReadRulerSummaryModel(QObject* parent):
    QStandardItemModel(parent)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
        tr("附加类型"),tr("采信数据"),tr("有效数"),tr("使用")
        });
}

void ReadRulerSummaryModel::setupModel(const readruler::IntervalReport& itrep)
{
    using SI = QStandardItem;
    setRowCount(static_cast<int>( itrep.types.size()));
    int row = 0;
    for (auto p = itrep.types.begin(); p != itrep.types.end(); ++p) {
        setItem(row, ColType, new SI(TrainLine::attachTypeStringFull(p->first)));
        setItem(row, ColValue, new SI(qeutil::secsDiffToString(p->second.value)));
        setItem(row, ColCount, new SI(QString::number(p->second.tot)));
        setItem(row, ColUsed, new SI(
            p->second.used ? tr("√") : tr("×")
        ));
        row++;
    }
}

#endif

