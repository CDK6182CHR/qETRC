#include "intervalcountdialog.h"

#include <dialogs/trainfilter.h>

#include <util/combos/railstationcombo.h>
#include <data/diagram/diagram.h>
#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QHeaderView>
#include <QTableView>
#include <data/common/qesystem.h>

IntervalCountModel::IntervalCountModel(QObject *parent):
    QStandardItemModel(parent)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels(
                {tr("发站"),tr("到站"),tr("车次数"),tr("始发数"),tr("终到数"),
                 tr("始发终到数")});
}

void IntervalCountModel::resetData(RailIntervalCount &&data_)
{
    _data=std::move(data_);
    setupModel();
}

void IntervalCountModel::refreshData()
{
    setupModel();
}

void IntervalCountModel::setupModel()
{
    using SI=QStandardItem;
    // TODO here 2022.4.17
}

IntervalCountDialog::IntervalCountDialog(Diagram &diagram, QWidget *parent):
    QDialog(parent), diagram(diagram),
    filter(new TrainFilter(diagram,this)),
    model(new IntervalCountModel(this)),
    counter(diagram.trainCollection(),filter->getCore())
{
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void IntervalCountDialog::initUI()
{
    setWindowTitle(tr("区间对数表"));
    resize(800,800);

    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;

    auto* lab=new QLabel(tr("此功能给出指定线路中，指定站到其他所有（符合条件）车站的对数。"
        "只有在两站都有数据（图定时刻）的列车才会被显示，即缺失数据的跨越车站不会被推定。"
        "如果需要相邻区间的、考虑跨越车站的对数表，请使用[断面对数]功能。\n"
        "双击各行显示对应的车次表。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    cbStation=new RailStationCombo(diagram.railCategory());
    flay->addRow(tr("查询车站"),cbStation);
    connect(cbStation,&RailStationCombo::stationChanged,
            this,&IntervalCountDialog::refreshData);

    rdStart=new RadioButtonGroup<2>({"作为出发站","作为到达站"},this);
    flay->addRow(tr("查询方式"),rdStart);
    rdStart->get(0)->setChecked(true);
    connect(rdStart->get(0),&QRadioButton::toggled,
            this,&IntervalCountDialog::refreshData);

    auto* hlay=new QHBoxLayout;
    ckPassenger=new QCheckBox(tr("仅显示办客车站"));
    ckFreight=new QCheckBox(tr("仅显示办货车站"));
    hlay->addWidget(ckPassenger);
    hlay->addWidget(ckFreight);
    flay->addRow(tr("车站筛选"),hlay);
    connect(ckPassenger,&QCheckBox::toggled,
            this,&IntervalCountDialog::refreshData);
    connect(ckFreight,&QCheckBox::toggled,
            this,&IntervalCountDialog::refreshData);

    hlay=new QHBoxLayout;
    ckBusiness=new QCheckBox(tr("仅营业车次"));
    ckStop=new QCheckBox(tr("仅停车车次"));
    auto* btn=new QPushButton(tr("车次筛选器"));
    hlay->addWidget(ckBusiness);
    hlay->addWidget(ckStop);
    hlay->addWidget(btn);
    connect(ckBusiness,&QCheckBox::toggled,
            this,&IntervalCountDialog::refreshData);
    connect(ckStop,&QCheckBox::toggled,
            this,&IntervalCountDialog::refreshData);
    connect(filter,&TrainFilter::filterApplied,
            this,&IntervalCountDialog::refreshData);
    connect(btn,&QPushButton::clicked,
            filter,&TrainFilter::show);
    flay->addRow(tr("车次筛选"),hlay);

    vlay->addLayout(flay);

    table=new QTableView;
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    connect(table,&QTableView::doubleClicked,
            this,&IntervalCountDialog::onDoubleClicked);
    {
        int c=0;
        // todo: 列宽
    }

    vlay->addWidget(table);

    auto* g=new ButtonGroup<2>({"导出CSV","关闭"});
    g->connectAll(SIGNAL(clicked()),this,
                  {SLOT(toCsv()),SLOT(close())});
    vlay->addLayout(g);

    refreshData();
}

void IntervalCountDialog::refreshData()
{
    counter.setBusinessOnly(ckBusiness->isChecked());
    counter.setStopOnly(ckStop->isChecked());
    counter.setPassengerOnly(ckPassenger->isChecked());
    counter.setFreightOnly(ckFreight->isChecked());

    RailIntervalCount data;
    if (rdStart->get(0)->isChecked()){
        // 单源点
        data=counter.getIntervalCountSource(cbStation->railway(),
                                            cbStation->station());
    }else{
        data=counter.getIntervalCountDrain(cbStation->railway(),
                                           cbStation->station());
    }
    model->resetData(std::move(data));
}

void IntervalCountDialog::onDoubleClicked()
{
    // todo ..
}

void IntervalCountDialog::toCsv()
{
    // todo ..
}


