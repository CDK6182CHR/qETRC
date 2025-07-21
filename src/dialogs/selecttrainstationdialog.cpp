#include "selecttrainstationdialog.h"

#include <model/train/timetablestdmodel.h>

#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDialogButtonBox>
#include "util/selecttraincombo.h"
#include "data/common/qesystem.h"
#include "util/utilfunc.h"
#include "data/train/train.h"
#include "model/delegate/traintimedelegate.h"

SelectTrainStationsDialog::SelectTrainStationsDialog(TrainCollection& coll, const DiagramOptions& ops,
                                                     QWidget *parent):
    QDialog(parent), coll(coll), _ops(ops), model(new TimetableStdModel(_ops, true, this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(700,600);
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

SelectTrainStationsDialog::result_type
    SelectTrainStationsDialog::dlgGetStation(TrainCollection &coll, const DiagramOptions& ops, QWidget *parent)
{
    auto* dlg=new SelectTrainStationsDialog(coll, ops, parent);
    dlg->setAttribute(Qt::WA_DeleteOnClose, false);

    auto flag = dlg->exec();
    result_type res;
    if (flag){
        res = dlg->getSelection();
    }
    dlg->setParent(nullptr);
    dlg->deleteLater();
    return res;
}

void SelectTrainStationsDialog::initUI()
{
    setWindowTitle(tr("选择时刻表车站"));
    auto* vlay=new QVBoxLayout(this);
    cbTrain=new SelectTrainCombo(coll);
    vlay->addLayout(cbTrain);

    auto* lab=new QLabel(tr("请在下表选择车站: "));
    vlay->addWidget(lab);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    {
        int c = 0;
        for (int w : {100, 90, 90, 40, 60, 60, 60}) {
            table->setColumnWidth(c++, w);
        }
    }
    auto* dele = new TrainTimeDelegate(_ops, this);
    table->setItemDelegateForColumn(TimetableStdModel::ColArrive, dele);
    table->setItemDelegateForColumn(TimetableStdModel::ColDepart, dele);
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->setSelectionMode(QTableView::MultiSelection);
    table->setSelectionBehavior(QTableView::SelectRows);
    connect(cbTrain,&SelectTrainCombo::currentTrainChanged,
            model, &TimetableStdModel::setTrain);
    vlay->addWidget(table);

    auto* box=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    vlay->addWidget(box);
    connect(box, &QDialogButtonBox::accepted,
            this,&SelectTrainStationsDialog::accept);
    connect(box, &QDialogButtonBox::rejected,
            this,&QDialog::close);
}
