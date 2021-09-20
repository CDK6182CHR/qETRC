#include "correcttimetabledialog.h"
#include "data/train/train.h"
#include "model/delegate/qedelegate.h"
#include "util/utilfunc.h"
#include "util/buttongroup.hpp"
#include "data/common/qesystem.h"

#include <QLabel>
#include <QTableView>
#include <QHeaderView>
#include <QFormLayout>

#include <util/selecttraincombo.h>

CorrectTimetableModel::CorrectTimetableModel(std::shared_ptr<Train> train, QObject *parent):
    QEMoveableModel(parent),train(train)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
        tr("站名"),tr("到点"),tr("开点"),tr("停时"),tr("区间"),tr("备注")
        });
    setupModel();
}

void CorrectTimetableModel::refreshData()
{
    setupModel();
}

void CorrectTimetableModel::doExchange()
{
    bool chg=false;
    for(int i=0;i<rowCount();i++){
        if (item(i, ColName)->checkState() == Qt::Checked){
            auto* it1=takeItem(i,ColArrive);
            auto* it2=takeItem(i,ColDepart);
            setItem(i,ColArrive,it2);
            setItem(i,ColDepart,it1);
            calculateDurations(i);
            chg=true;
        }else{
            if (chg){
                // 表明上一行发生了变化
                calculateDurations(i);
                chg=false;
            }
        }
    }
}

void CorrectTimetableModel::doReverse()
{
    // todo ..
}

void CorrectTimetableModel::doMoveUp()
{
    // todo ..
}

void CorrectTimetableModel::doMoveDown()
{
    // todo ..
}

void CorrectTimetableModel::doToTop()
{
    // todo ..
}

void CorrectTimetableModel::doToBottom()
{
    // todo ..
}

void CorrectTimetableModel::selectAll()
{
    // todo ..
}

void CorrectTimetableModel::deselectAll()
{
    // todo ..
}

void CorrectTimetableModel::selectInverse()
{
    // todo ..
}

//void CorrectTimetableModel::setTrain(std::shared_ptr<Train> train)
//{
//    this->train = train;
//    refreshData();
//}

void CorrectTimetableModel::setupModel()
{
    if (!train) {
        setRowCount(0);
        return;
    }
    using SI = QStandardItem;
    setRowCount(train->stationCount());
    int row = 0;
    for (auto p = train->timetable().begin(); p != train->timetable().end(); ++p) {
        auto* it = new SI(p->name.toSingleLiteral());
        it->setCheckable(true);
        QVariant v;
        v.setValue(p);
        it->setData(v, qeutil::TrainStationRole);
        setItem(row, ColName, it);

        it = new SI;
        it->setData(p->arrive, Qt::EditRole);
        setItem(row, ColArrive, it);
        it = new SI;
        it->setData(p->depart, Qt::EditRole);
        setItem(row, ColDepart, it);

        setItem(row, ColNote, new SI(p->note));

        calculateDurations(row);
        ++row;
    }
}

void CorrectTimetableModel::calculateDurations(int row)
{
    using SI = QStandardItem;
    const QTime& arr = rowArrive(row), & dep = rowDepart(row);

    // 停时
    int secs = qeutil::secsTo(arr, dep);
    auto* it = new SI(qeutil::secsToString(secs));
    if (secs > 12 * 3600) {
        it->setBackground(QColor(255, 0, 0, 150));
    }
    setItem(row, ColStopDuration, it);

    if (row > 0) {
        const QTime& last = rowDepart(row - 1);
        secs = qeutil::secsTo(last, arr);
        it = new SI(qeutil::secsToString(secs));
        if (secs > 12 * 3600) {
            it->setBackground(QColor(255, 0, 0, 150));
        }
        setItem(row, ColIntervalDuration, it);
    }
}

QTime CorrectTimetableModel::rowArrive(int row) const
{
    return item(row, ColArrive)->data(Qt::EditRole).toTime();
}

QTime CorrectTimetableModel::rowDepart(int row) const
{
    return item(row, ColDepart)->data(Qt::EditRole).toTime();
}

CorrectTimetableDialog::CorrectTimetableDialog(
                 std::shared_ptr<Train> train, QWidget *parent):
    QDialog(parent),train(train), model(new CorrectTimetableModel(train,this))
{
    resize(700,800);
    setWindowTitle(tr("时刻表修正 - %1").arg(train->trainName().full()));
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void CorrectTimetableDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr(""));

    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* g4=new ButtonGroup<4>({"上移","下移","置顶","置底"});
    vlay->addLayout(g4);
    g4->connectAll(SIGNAL(clicked()),model,{SLOT(doMoveUp()),SLOT(doMoveDown()),
                   SLOT(doToTop()),SLOT(doToBottom())});
    auto* g2=new ButtonGroup<2>({"交换到发","区间反排"});
    vlay->addLayout(g2);
    g2->connectAll(SIGNAL(clicked()),model,{SLOT(doExchange()),SLOT(doReverse())});
    g4=new ButtonGroup<4>({"全选","全不选","反选","批选"});
    vlay->addLayout(g4);
    g4->connectFront(SIGNAL(clicked()),model,{SLOT(selectAll()),SLOT(deselectAll()),
                     SLOT(selectInverse())});
    connect(g4->get(3),&QPushButton::clicked,
            this,&CorrectTimetableDialog::batchSelect);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->setEditTriggers(QTableView::NoEditTriggers);
    vlay->addWidget(table);

    int c = 0;
    for (int i : {100, 100, 100, 80, 80, 80}) {
        table->setColumnWidth(c++, i);
    }

    auto* g3=new ButtonGroup<3>({"确定","还原","取消"});
    g3->connectAll(SIGNAL(clicked()),this,{SLOT(actApply()),SLOT(refreshData()),
                   SLOT(close())});
    vlay->addLayout(g3);
}

void CorrectTimetableDialog::actApply()
{
    //todo
}

void CorrectTimetableDialog::refreshData()
{
    model->refreshData();
}

void CorrectTimetableDialog::batchSelect()
{
    const auto& sel=table->selectionModel()->selectedRows();
    foreach(const auto& idx, sel){
        auto* it=model->item(idx.row(),CorrectTimetableModel::ColName);
        if (it->checkState() == Qt::Checked){
            it->setCheckState(Qt::Unchecked);
        }else{
            it->setCheckState(Qt::Checked);
        }
    }
}

//void CorrectTimetableDialog::setTrain(std::shared_ptr<Train> train)
//{
//    this->train=train;
//}
