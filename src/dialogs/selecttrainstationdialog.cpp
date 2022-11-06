#include "selecttrainstationdialog.h"

#include <model/train/timetablestdmodel.h>

#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <util/selecttraincombo.h>
#include <data/common/qesystem.h>
#include <util/utilfunc.h>
#include <data/train/train.h>

SelectTrainStationsDialog::SelectTrainStationsDialog(TrainCollection& coll,
                                                     QWidget *parent):
    QDialog(parent), coll(coll), model(new TimetableStdModel(true, this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(500,600);
    initUI();
    refreshData();
}

std::vector<std::list<TrainStation>::iterator>
    SelectTrainStationsDialog::getSelection()
{
    auto train=cbTrain->train();
    if (!train) return {};
    std::vector<std::list<TrainStation>::iterator> res;
    const auto& sel=table->selectionModel()->selectedRows();
    auto rows = qeutil::indexRows(sel);

    int i=0;
    for (auto itr=train->timetable().begin();itr!=train->timetable().end();++itr,++i){
        if (auto p=rows.find(i);p!=rows.end()){
            // this row is selected
            res.emplace_back(itr);
            rows.erase(p);
        }
    }
    return res;
}

void SelectTrainStationsDialog::refreshData()
{
    model->setTrain(cbTrain->train());
}

void SelectTrainStationsDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    cbTrain=new SelectTrainCombo(coll);
    vlay->addLayout(cbTrain);

    auto* lab=new QLabel(tr("请在下表选择车站: "));
    vlay->addWidget(lab);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->setSelectionMode(QTableView::MultiSelection);
    table->setSelectionBehavior(QTableView::SelectRows);
    connect(cbTrain,&SelectTrainCombo::currentTrainChanged,
            model, &TimetableStdModel::setTrain);
    vlay->addWidget(table);
}
