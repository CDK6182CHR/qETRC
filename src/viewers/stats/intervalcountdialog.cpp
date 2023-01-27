#include "intervalcountdialog.h"

#include <util/combos/railstationcombo.h>
#include <data/diagram/diagram.h>
#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QHeaderView>
#include <QTableView>
#include <data/common/qesystem.h>
#include <model/delegate/qedelegate.h>
#include <util/utilfunc.h>
#include <editors/train/trainfilterselector.h>
#include "data/rail/railway.h"
#include "intervaltraintable.h"

IntervalCountModel::IntervalCountModel(IntervalCounter& counter, QObject *parent):
    QStandardItemModel(parent),counter(counter)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels(
                {tr("发站"),tr("到站"),tr("车次数"),tr("始发数"),tr("终到数"),
                 tr("始发终到数")});
}

void IntervalCountModel::resetData(RailIntervalCount &&data_, std::shared_ptr<const RailStation> center, bool isStart,
    std::shared_ptr<const Railway> rail)
{
    _data=std::move(data_);
    this->center = center;
    this->isStart = isStart;
    this->railway = rail;
    setupModel();
}

std::shared_ptr<const RailStation> IntervalCountModel::stationForRow(int i) const
{
    return railway->stations().at(i);
}

void IntervalCountModel::refreshData()
{
    setupModel();
}

void IntervalCountModel::setupModel()
{
    using SI=QStandardItem;
    setRowCount(railway->stations().size());
    int row = 0;
    foreach(auto st , railway->stations()){

        auto* it_center = new SI(center->name.toSingleLiteral());
        auto* it_edge = new SI(st->name.toSingleLiteral());

        if (isStart) {
            setItem(row, ColFrom, it_center);
            setItem(row, ColTo, it_edge);
        }
        else {
            setItem(row, ColTo, it_center);
            setItem(row, ColFrom, it_edge);
        }

        if (auto itr = _data.find(st); itr != _data.end()) {
            // 有数据
            const auto& info = itr->second;
            setItem(row, ColTotal, new SI(QString::number(info.count())));
            if (int t = info.startCount()) {
                setItem(row, ColStart, new SI(QString::number(t)));
            }
            else {
                setItem(row, ColStart, new SI("-"));
            }
            if (int t = info.endCount()) {
                setItem(row, ColEnd, new SI(QString::number(t)));
            }
            else {
                setItem(row, ColEnd, new SI("-"));
            }
            if (int t = info.startEndCount()) {
                setItem(row, ColStartEnd, new SI(QString::number(t)));
            }
            else {
                setItem(row, ColStartEnd, new SI("-"));
            }
        }
        else {
            for (int c = ColTotal; c <= ColStartEnd; c++) {
                setItem(row, c, new SI("-"));
            }
        }
        row++;
    }
}

IntervalCountDialog::IntervalCountDialog(Diagram &diagram, QWidget *parent):
    QDialog(parent), diagram(diagram),
    filter(new TrainFilterSelector(diagram.trainCollection(),this)),
    counter(diagram.trainCollection()),
    model(new IntervalCountModel(counter, this))
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
            this,&IntervalCountDialog::refreshShow);
    connect(ckFreight,&QCheckBox::toggled,
            this,&IntervalCountDialog::refreshShow);

    hlay=new QHBoxLayout;
    ckBusiness=new QCheckBox(tr("仅营业车次"));
    ckStop=new QCheckBox(tr("仅停车车次"));
    hlay->addWidget(ckBusiness);
    hlay->addWidget(ckStop);
    hlay->addWidget(filter);
    connect(ckBusiness,&QCheckBox::toggled,
            this,&IntervalCountDialog::refreshData);
    connect(ckStop,&QCheckBox::toggled,
            this,&IntervalCountDialog::refreshData);
    connect(filter,&TrainFilterSelector::filterChanged,
            this,&IntervalCountDialog::refreshData);
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
        for (int w : {100, 100, 80, 80, 80, 80}) {
            table->setColumnWidth(c++, w);
        }
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
    counter.setFilter(filter->filter());

    RailIntervalCount data;
    if (rdStart->get(0)->isChecked()){
        // 单源点
        data=counter.getIntervalCountSource(cbStation->railway(),
                                            cbStation->station());
    }else{
        data=counter.getIntervalCountDrain(cbStation->railway(),
                                           cbStation->station());
    }
    model->resetData(std::move(data), cbStation->station(),rdStart->get(0)->isChecked(),
        cbStation->railway());
    refreshShow();
}

void IntervalCountDialog::refreshShow()
{
    auto rail = cbStation->railway();
    if (!rail) return;
    for (int i = 0; i < model->rowCount(); i++) {
        auto st = rail->stations().at(i);
        table->setRowHidden(i, !counter.checkStation(st));
    }
}

void IntervalCountDialog::onDoubleClicked()
{
    int i = table->currentIndex().row();
    if (i < 0)return;

    if (detailTable == nullptr) {
        detailTable = new IntervalTrainTable(this);
        detailTable->setWindowFlag(Qt::Dialog);
        detailTable->resize(700, 700);
    }
    
    const auto& data = model->getData();
    auto st = model->stationForRow(i);
    if (auto itr = data.find(st); itr != data.end()) {
        detailTable->getModel()->resetData(itr->second.list());
    }
    else {
        detailTable->getModel()->resetData({});
    }

    auto center = cbStation->station();
    // 标题
    if (rdStart->get(0)->isChecked()) {
        detailTable->setWindowTitle(tr("区间车次表 [%1->%2]").arg(center->name.toSingleLiteral(),
            st->name.toSingleLiteral()));
    }
    else {
        detailTable->setWindowTitle(tr("区间车次表 [%2->%1]").arg(center->name.toSingleLiteral(),
            st->name.toSingleLiteral()));
    }

    detailTable->show();
}

void IntervalCountDialog::toCsv()
{
    auto rail = cbStation->railway();
    if (!rail)return;
    qeutil::exportTableToCsv(model, this,
        tr("%1区间对数表").arg(rail->name()));
}


