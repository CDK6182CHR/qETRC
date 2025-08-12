#include "correcttimetabledialog.h"
#include "data/train/train.h"
#include "model/delegate/qedelegate.h"
#include "util/utilfunc.h"
#include "util/buttongroup.hpp"
#include "data/common/qesystem.h"
#include "model/delegate/traintimedelegate.h"

#include <QLabel>
#include <QTableView>
#include <QHeaderView>
#include <QFormLayout>
#include <QBitArray>
#include <QCheckBox>
#include <QMessageBox>
#include <stdexcept>

#include "util/selecttraincombo.h"
#include "data/algo/timetablecorrector.h"
#include "data/diagram/diagramoptions.h"

CorrectTimetableModel::CorrectTimetableModel(const DiagramOptions& ops, std::shared_ptr<Train> train, QObject *parent):
    QEMoveableModel(parent), _ops(ops), train(train)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
        tr("站名"),tr("到点"),tr("开点"),tr("停时"),tr("区间"),tr("营业"),tr("股道"),
                                  tr("备注")
        });
    setupModel();
    connect(this, &QEMoveableModel::dataChanged,
        this, &CorrectTimetableModel::onDataChanged);
}

void CorrectTimetableModel::refreshData()
{
    setupModel();
}

std::shared_ptr<Train> CorrectTimetableModel::appliedTrain()
{
    auto res=std::make_shared<Train>(train->trainName());
    for(int i=0;i<rowCount();i++){
        res->appendStation(item(i,ColName)->text(),
                           rowArrive(i),rowDepart(i),
                           item(i,ColBusiness)->checkState()==Qt::Checked,
                           item(i,ColTrack)->text(),
                           item(i,ColNote)->text());
    }
    return res;
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
    int first = firstSelectedRow(), last = lastSelectedRow();
    if (first == -1)return;
    int front = first, back = last;
    while (front < back) {
        exchangeRow(front++, back--);
    }
    for (int row = first; row <= last + 1 && row < rowCount(); row++) {
        calculateDurations(row);
    }
}

void CorrectTimetableModel::doMoveUp()
{
    QBitArray ar = QBitArray(rowCount());
    for (int i = 1; i < rowCount(); i++) {
        // 第一行不用管..
        if (item(i, ColName)->checkState() == Qt::Checked) {
            moveUp(i);
            ar.setBit(i); ar.setBit(i - 1); 
            [[likely]] if (i < rowCount() - 1)
                ar.setBit(i + 1);
        }
    }
    calculateSelectedRows(ar);
}

void CorrectTimetableModel::doMoveDown()
{
    QBitArray ar(rowCount() + 1);
    for (int i = rowCount() - 2; i >= 0; --i) {
        if (item(i, ColName)->checkState() == Qt::Checked) {
            moveDown(i);
            ar.setBit(i); ar.setBit(i + 1); ar.setBit(i + 2);
        }
    }
    calculateSelectedRows(ar);
}

void CorrectTimetableModel::doToTop()
{
    int r = 0;
    for (int i = 0; i < rowCount(); i++) {
        if (item(i, ColName)->checkState() == Qt::Checked) {
            moveRow({}, i, {}, r++);
        }
    }
    calculateAllRows();
}

void CorrectTimetableModel::doToBottom()
{
    int r = rowCount();
    // 倒着遍历
    for (int i = rowCount() - 1; i >= 0; i--) {
        if (item(i, ColName)->checkState() == Qt::Checked) {
            moveRow({}, i, {}, r--);
        }
    }
    calculateAllRows();
}

void CorrectTimetableModel::doPartialSortArrive()
{
    partialSort(&TrainStation::ltArrive);
}

void CorrectTimetableModel::doPartialSortDepart()
{
    partialSort(&TrainStation::ltDepart);
}

void CorrectTimetableModel::selectAll()
{
    for (int i = 0; i < rowCount(); i++) {
        item(i, ColName)->setCheckState(Qt::Checked);
    }
}

void CorrectTimetableModel::deselectAll()
{
    for (int i = 0; i < rowCount(); i++) {
        item(i, ColName)->setCheckState(Qt::Unchecked);
    }
}

void CorrectTimetableModel::selectInverse()
{
    for (int i = 0; i < rowCount(); i++) {
        auto* it = item(i, ColName);
        if (it->checkState() == Qt::Checked) {
            it->setCheckState(Qt::Unchecked);
        }
        else {
            it->setCheckState(Qt::Checked);
        }
    }
}

void CorrectTimetableModel::setTrain(std::shared_ptr<Train> train)
{
    this->train=train;
    refreshData();
}

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
        it->setData(QVariant::fromValue(p), qeutil::TrainStationRole);
        setItem(row, ColName, it);

        it = new SI;
        it->setData(p->arrive.toQVariant(), Qt::EditRole);
        setItem(row, ColArrive, it);
        it = new SI;
        it->setData(p->depart.toQVariant(), Qt::EditRole);
        setItem(row, ColDepart, it);

        it=makeCheckItem();
        it->setCheckState(qeutil::boolToCheckState(p->business));
        setItem(row,ColBusiness,it);
        setItem(row,ColTrack,new SI(p->track));
        setItem(row, ColNote, new SI(p->note));

        calculateDurations(row);
        ++row;
    }
}

void CorrectTimetableModel::calculateDurations(int row)
{
    using SI = QStandardItem;
    const TrainTime& arr = rowArrive(row), & dep = rowDepart(row);

    // 停时
    int secs = qeutil::secsTo(arr, dep, _ops.period_hours);
    auto* it = new SI(qeutil::secsToStringWithEmpty(secs));
    if (secs > 12 * 3600) {
        it->setBackground(QColor(255, 0, 0, 150));
    }
    setItem(row, ColStopDuration, it);

    if (row > 0) {
        const TrainTime& last = rowDepart(row - 1);
        secs = qeutil::secsTo(last, arr, _ops.period_hours);
        it = new SI(qeutil::secsToString(secs));
        if (secs > 12 * 3600) {
            it->setBackground(QColor(255, 0, 0, 150));
        }
        setItem(row, ColIntervalDuration, it);
    }
    else {
        setItem(row, ColIntervalDuration, new SI);
    }
}

TrainTime CorrectTimetableModel::rowArrive(int row) const
{
    return TrainTime::fromQVariant(item(row, ColArrive)->data(Qt::EditRole));
}

TrainTime CorrectTimetableModel::rowDepart(int row) const
{
    return TrainTime::fromQVariant(item(row, ColDepart)->data(Qt::EditRole));
}

void CorrectTimetableModel::calculateSelectedRows(const QBitArray &rows)
{
    for (int i = 0; i < rowCount(); i++) {
        if (rows.testBit(i)) {
            calculateDurations(i);
        }
    }
}

void CorrectTimetableModel::calculateAllRows()
{
    for (int i = 0; i < rowCount(); i++) {
        calculateDurations(i);
    }
}

int CorrectTimetableModel::firstSelectedRow()
{
    for (int i = 0; i < rowCount(); i++) {
        if (item(i, ColName)->checkState() == Qt::Checked) {
            return i;
        }
    }
    return -1;
}

int CorrectTimetableModel::lastSelectedRow()
{
    for (int i = rowCount() - 1; i >= 0; i--) {
        if (item(i, ColName)->checkState() == Qt::Checked) {
            return i;
        }
    }
    return -1;
}

void CorrectTimetableModel::partialSort(bool(*comp)(const TrainStation&, const TrainStation&))
{
    int first = firstSelectedRow(), last = lastSelectedRow();
    if (first == -1)return;

    auto nt = appliedTrain();
    auto itr_first = nt->timetable().begin(); std::advance(itr_first, first);
    auto itr_last = nt->timetable().begin(); std::advance(itr_last, last + 1);

    std::list<TrainStation> tmp;
    tmp.splice(tmp.begin(), nt->timetable(), itr_first, itr_last);
    tmp.sort(comp);

    nt->timetable().splice(itr_last, std::move(tmp));

    setTrain(nt);
}



void CorrectTimetableModel::onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    if (!roles.contains(Qt::EditRole))
        return;
    if (updating)
        return;
    if (std::max(topLeft.column(), (int)ColArrive) <=
        std::min(bottomRight.column(), (int)ColDepart)) {
        for (int r = topLeft.row(); r <= bottomRight.row(); r++) {
            calculateDurations(r);
        }
    }
}

CorrectTimetableDialog::CorrectTimetableDialog(
                 const DiagramOptions& ops, std::shared_ptr<Train> train, QWidget *parent):
    QDialog(parent), _ops(ops), train(train), model(new CorrectTimetableModel(_ops, train, this))
{
    resize(700,800);
    setWindowTitle(tr("时刻表修正 - %1").arg(train->trainName().full()));
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void CorrectTimetableDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("本功能提供时刻表排序中常见问题的手动更正功能。"
        "请先选择要操作的行，然后选择相应的操作。\n"
        "说明：上移（下移）功能将所有选中的行从当前位置向上（向下）移动一行；"
        "置顶、置底功能保持当前选中的行的顺序不变，而将它们移动到时刻表最前或者最后；"
        "反排功能将当前选中的第一行和最后一行之间的所有行顺序反排。"));

    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* flay=new QFormLayout;
    ckEdit=new QCheckBox(tr("启用全部编辑"));
    connect(ckEdit,&QCheckBox::toggled,this,&CorrectTimetableDialog::onEditToggled);
    flay->addRow(tr("选项"),ckEdit);
    vlay->addLayout(flay);

    auto* g5=new ButtonGroup<5>({"上移","下移","置顶","置底","自动更正 (测试)" });
    vlay->addLayout(g5);
    g5->connectFront(SIGNAL(clicked()),model,{SLOT(doMoveUp()),SLOT(doMoveDown()),
                   SLOT(doToTop()),SLOT(doToBottom())});
    connect(g5->get(4), &QPushButton::clicked, this, &CorrectTimetableDialog::autoCorrect);

    auto* g2=new ButtonGroup<4>({"交换到发","区间反排","到点排序","发点排序"});
    vlay->addLayout(g2);
    g2->connectAll(SIGNAL(clicked()),model,{SLOT(doExchange()),SLOT(doReverse()),
        SLOT(doPartialSortArrive()), SLOT(doPartialSortDepart())});
    
    auto* g4=new ButtonGroup<4>({"全选","全不选","反选","批选"});
    vlay->addLayout(g4);
    g4->connectFront(SIGNAL(clicked()),model,{SLOT(selectAll()),SLOT(deselectAll()),
                     SLOT(selectInverse())});
    connect(g4->get(3),&QPushButton::clicked,
            this,&CorrectTimetableDialog::batchSelect);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->setSelectionMode(QTableView::ExtendedSelection);
    vlay->addWidget(table);
    auto* dele = new TrainTimeDelegate(_ops, this);
    table->setItemDelegateForColumn(CorrectTimetableModel::ColArrive, dele);
    table->setItemDelegateForColumn(CorrectTimetableModel::ColDepart, dele);

    int c = 0;
    for (int i : {120, 80, 80, 80, 80, 30, 80, 80}) {
        table->setColumnWidth(c++, i);
    }

    auto* g3=new ButtonGroup<3>({"确定","还原","取消"});
    g3->connectAll(SIGNAL(clicked()),this,{SLOT(actApply()),SLOT(refreshData()),
                   SLOT(close())});
    vlay->addLayout(g3);
}

void CorrectTimetableDialog::actApply()
{
    auto res = model->appliedTrain();
    emit correctionApplied(train, res);
}

void CorrectTimetableDialog::onEditToggled(bool on)
{
    if (on){
        if(informEdit){
            QMessageBox::information(this,tr("提示"),tr("本功能主要设计为"
            "通过下面的按钮对时刻表进行重排，以修复时刻表中常见的错误类型。"
            "但方便起见，仍然提供对时刻表既有车站全部元素进行编辑的功能。"
            "请注意防止误触。\n此消息在本功能每次运行期间，提示一次。"));
            informEdit=false;
        }
        table->setEditTriggers(QTableView::AllEditTriggers);
    }else{
        table->setEditTriggers(QTableView::NoEditTriggers);
    }
}

void CorrectTimetableDialog::autoCorrect()
{
    if(!train)
        return;
    auto res=QMessageBox::question(this,tr("自动时刻更正"),
                                   tr("此功能按一定算法尝试更正时刻表中可能存在的顺序问题。"
                                    "由于算法较老，未经充分测试，不一定能解决所有问题。"
                                    "建议在调用此功能之前做好数据保存与备份。"
                                    "是否确认执行？"));
    if (res!=QMessageBox::Yes)
        return;

    auto nt=model->appliedTrain();
    //try{
        bool flag=TimetableCorrector::autoCorrect(nt, _ops.period_hours);
        if (flag){
            model->setTrain(nt);
            QMessageBox::information(this,tr("提示"),tr("自动更正执行成功，已更新表格"));
        }else{
            QMessageBox::information(this,tr("提示"),tr("自动更正未能修改时刻表"));
        }
    //}catch(const std::exception& e){
    //    QMessageBox::warning(this,tr("错误"),
    //                         tr("程序内部错误：%1").arg(e.what()));
    //}

}

void CorrectTimetableDialog::refreshData()
{
    model->setTrain(train);
}

void CorrectTimetableDialog::batchSelect()
{
    const auto& sel = table->selectionModel()->selectedIndexes();
    // 转换成行的信息
    QSet<int> rows;
    foreach(const auto & idx, sel) {
        rows.insert(idx.row());
    }
    foreach(int row,rows){
        auto* it=model->item(row,CorrectTimetableModel::ColName);
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
