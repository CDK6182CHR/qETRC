#include "greedypaintpagepaint.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QTimeEdit>
#include <QMessageBox>
#include <QLabel>
#include <QTextBrowser>

#include <data/train/trainname.h>
#include <data/common/qesystem.h>
#include <data/rail/ruler.h>
#include <data/rail/rulernode.h>
#include <model/delegate/qedelegate.h>
#include <model/delegate/generalspindelegate.h>
#include <model/delegate/postivespindelegate.h>
#include <model/delegate/qetimedelegate.h>
#include <model/delegate/timeintervaldelegate.h>
#include <data/calculation/greedypainter.h>
#include <data/diagram/diagram.h>
#include <data/train/train.h>


GreedyPaintConfigModel::GreedyPaintConfigModel(QWidget* parent):
    QStandardItemModel(parent)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({ tr("站名"),tr("锚"),tr("起"),tr("止"),tr("停分"),tr("停秒"),
        tr("实停"),tr("到达时刻"),tr("出发时刻") });
    connect(this, &GreedyPaintConfigModel::dataChanged,
        this, &GreedyPaintConfigModel::onDataChanged);
}

void GreedyPaintConfigModel::setRuler(std::shared_ptr<Ruler> ruler)
{
    _ruler = ruler;
}

int GreedyPaintConfigModel::startRow()const
{
    return _startRow >= 0 ? _startRow : _anchorRow;
}

int GreedyPaintConfigModel::endRow()const
{
    return _endRow >= 0 ? _endRow : _anchorRow;
}

void GreedyPaintConfigModel::refreshData()
{
    if (!_ruler) {
        setRowCount(0);
        return;
    }
    //blockSignals(true);

    using SI = QStandardItem;
    auto rail = _ruler->railway();
    setRowCount(rail->stationCount());
    if (rail->empty()) return;

    int row = 0;
    for (auto st=rail->firstDirStation(_dir);st;st=st->dirAdjacent(_dir)) {

        if (!st->isDirectionVia(_dir)) {
            continue;
        }

        auto* it = new SI(st->name.toSingleLiteral());
        QVariant v;
        v.setValue(st);
        it->setData(v, qeutil::RailStationRole);
        setItem(row, ColName, it);
        it->setEditable(false);

        it = makeCheckItem();
        setItem(row, ColAnchor, it);

        it = makeCheckItem();
        setItem(row, ColStart, it);

        it = makeCheckItem();
        setItem(row, ColEnd, it);

        it = new SI;
        it->setData(0, Qt::EditRole);
        setItem(row, ColMinute, it);

        it = new SI;
        it->setData(0, Qt::EditRole);
        setItem(row, ColSecond, it);

        setItem(row, ColActualStop, makeReadonlyItem());
        setItem(row, ColArrive, makeReadonlyItem());
        setItem(row, ColDepart, makeReadonlyItem());

        row++;
    }
    setRowCount(row);
    //blockSignals(false);

    _anchorRow = _startRow = _endRow = -1;

    setAnchorRowNoSignal(0);

}

QStandardItem* GreedyPaintConfigModel::makeCheckItem()
{
    auto* it = new QStandardItem;
    it->setEditable(false);
    it->setCheckable(true);
    return it;
}

QStandardItem* GreedyPaintConfigModel::makeReadonlyItem()
{
    auto* it = new QStandardItem;
    it->setEditable(false);
    return it;
}

std::shared_ptr<const RailStation> GreedyPaintConfigModel::stationForRow(int row)const
{
    auto* it = item(row, ColName);
    return qvariant_cast<std::shared_ptr<const RailStation>>(it->data(qeutil::RailStationRole));
}

int GreedyPaintConfigModel::stopSecsForRow(int row) const
{
    int mins = item(row, ColMinute)->data(Qt::EditRole).toInt();
    int secs = item(row, ColSecond)->data(Qt::EditRole).toInt();
    return mins * 60 + secs;
}

void GreedyPaintConfigModel::setRowColor(int row, const QColor& color)
{
    item(row, ColName)->setForeground(color);
}

void GreedyPaintConfigModel::setRowBackground(int row, const QColor& color)
{
    for (int c = 0; c < ColMAX; c++) {
        item(row, c)->setBackground(color);
    }
}

int GreedyPaintConfigModel::rowForStation(const StationName& name)
{
    for (int i = 0; i < rowCount(); i++) {
        if (stationForRow(i)->name == name)
            return i;
    }
    qDebug() << "GreedyPaintConfigModel::rowForStation: ERROR: name not found! " 
        << name.toSingleLiteral() << Qt::endl;
    return -1;
}

void GreedyPaintConfigModel::setTimetable(std::shared_ptr<Train> train)
{
    if (!train || train->empty())
        return;
    int row = rowForStation(train->timetable().front().name);
    using SI = QStandardItem;
    for (const auto& st:train->timetable()) {
        item(row, ColActualStop)->setData(st.stopSec(), Qt::EditRole);
        item(row, ColArrive)->setData(st.arrive, Qt::EditRole);
        item(row, ColDepart)->setData(st.depart, Qt::EditRole);
        row++;
    }
}

std::shared_ptr<const RailStation> GreedyPaintConfigModel::startStation() const
{
    return stationForRow(startRow());
}

std::shared_ptr<const RailStation> GreedyPaintConfigModel::endStation() const
{
    return stationForRow(endRow());
}

std::shared_ptr<const RailStation> GreedyPaintConfigModel::anchorStation() const
{
    return stationForRow(anchorRow());
}

std::map<std::shared_ptr<const RailStation>, int> GreedyPaintConfigModel::stopSeconds() const
{
    std::map<std::shared_ptr<const RailStation>, int> res;
    for (int r = _availableFirstRow; r <= _availableLastRow; r++) {
        if (int secs = stopSecsForRow(r)) {
            res.emplace(stationForRow(r), secs);
        }
    }
    return res;
}

void GreedyPaintConfigModel::setStartRow(int row)
{
    //blockSignals(true);
    if (row == _startRow)
        return;
    if (_startRow >= 0) {
        item(_startRow, ColStart)->setCheckState(Qt::Unchecked);
    }
    item(row, ColStart)->setCheckState(Qt::Checked);
    _startRow = row;
    //blockSignals(false);
    emit startStationChanged(startStation());
}

void GreedyPaintConfigModel::setEndRow(int row)
{
    //blockSignals(true);
    if (row == _endRow)
        return;
    if (_endRow >= 0) {
        item(_endRow, ColEnd)->setCheckState(Qt::Unchecked);
    }
    item(row, ColEnd)->setCheckState(Qt::Checked);
    _endRow = row;
    //blockSignals(false);
    emit endStationChanged(endStation());
}

void GreedyPaintConfigModel::setAnchorRow(int row)
{
    setAnchorRowNoSignal(row);

    // 检查/处理始发终到站
    if (_startRow < _availableFirstRow) {
        setStartRow(_anchorRow);
    }
    if (_endRow<0 || _endRow>_availableLastRow) {
        setEndRow(_anchorRow);
    }

    emit anchorStationChanged(anchorStation());
}

void GreedyPaintConfigModel::setAnchorRowNoSignal(int row)
{
    //blockSignals(true);
    if (row == _anchorRow) {
        return;
    }
    if (_anchorRow >= 0)
        unsetAnchorRow(_anchorRow);
    _anchorRow = row;

    // 设置文字颜色
    auto st = stationForRow(row);
    setRowColor(row, Qt::black);

    // 回溯
    int r0 = row;
    auto it = st->dirPrevInterval(_dir);
    if (it) {
        auto node = it->getRulerNode(_ruler);

        for (; node; node = node->prevNode()) {
            if (node->isNull())
                break;
            else
                r0--;
            setRowColor(r0, Qt::black);
        }
        _availableFirstRow = r0;
        for (r0--; r0 >= 0; r0--) {
            setRowColor(r0, Qt::darkGray);
        }
    }
    else {
        _availableFirstRow = 0;
    }

    // 正推
    r0 = row;  // r0指向第一个不可用站
    it = st->dirNextInterval(_dir);
    if (it) {
        auto node = it->getRulerNode(_ruler);
        for (; node; node = node->nextNode()) {
            if (node->isNull())
                break;
            else
                r0++;
            setRowColor(r0, Qt::black);
        }
        _availableLastRow = r0;  // r0指向最后一个可用站
        for (r0++; r0 < rowCount(); r0++) {
            setRowColor(r0, Qt::darkGray);
        }
    }
    else {
        _availableLastRow = rowCount() - 1;
    }

    // 标记anchor站
    QColor color = Qt::blue;
    color.setAlpha(55);
    setRowBackground(row, color);
    item(row, ColAnchor)->setCheckState(Qt::Checked);
    //blockSignals(false);
}

void GreedyPaintConfigModel::onDataChanged(const QModelIndex& topLeft,
    const QModelIndex& bottomRight, const QVector<int>& roles)
{
    int c1 = topLeft.column(), c2 = bottomRight.column();
    for (int row = topLeft.row(); row <= bottomRight.row(); row++) {
        if (std::max(c1, (int)ColMinute) <= std::min(c2, (int)ColSecond)) {
            // 停车时间变化
            if (roles.contains(Qt::EditRole)) {
                onStopTimeChanged(row);
            }
        }

        if (roles.contains(Qt::CheckStateRole)) {
            // 三个勾选的变化了
            for (int c = c1; c <= c2; c++) {

                // 发生新勾选事件
                if (item(row, c)->checkState() == Qt::Checked) {
                    switch (c) {
                    case ColAnchor:setAnchorRow(row); break;
                    case ColStart:setStartRow(row); break;
                    case ColEnd:setEndRow(row); break;
                    }
                }
                else {
                    // 取消勾选事件，只处理start,end
                    switch (c) {
                    case ColStart:unsetStartRow(row); break;
                    case ColEnd:unsetEndRow(row); break;
                    }
                }
            }
        }
    }
}

void GreedyPaintConfigModel::onStopTimeChanged(int row)
{
    bool rangechanged = false;
    if (row < _anchorRow) {
        if (row >= _availableFirstRow && row < _startRow) {
            setStartRow(row);
            rangechanged = true;
        }
    }
    else if(row>_anchorRow) {
        if (row <= _availableLastRow && row > _endRow) {
            setEndRow(row);
            rangechanged = true;
        }
    }
    if (!rangechanged) {
        emit stopTimeChanged();
    }
}

void GreedyPaintConfigModel::unsetAnchorRow(int row)
{
    item(row, ColAnchor)->setCheckState(Qt::Unchecked);
    setRowBackground(row, Qt::transparent);
}

void GreedyPaintConfigModel::unsetStartRow(int row)
{
    _startRow = -1;
    emit startStationChanged(startStation());
}

void GreedyPaintConfigModel::unsetEndRow(int row)
{
    _endRow = -1;
    emit endStationChanged(endStation());
}


GreedyPaintPagePaint::GreedyPaintPagePaint(Diagram &diagram_,
                                           GreedyPainter &painter_,
                                           QWidget *parent):
    QWidget(parent),diagram(diagram_),painter(painter_),
    _model(new GreedyPaintConfigModel(this))
{
    initUI();
    connect(_model, &GreedyPaintConfigModel::startStationChanged,
        this, &GreedyPaintPagePaint::onStartChanged);
    connect(_model, &GreedyPaintConfigModel::endStationChanged,
        this, &GreedyPaintPagePaint::onEndChanged);
    connect(_model, &GreedyPaintConfigModel::anchorStationChanged,
        this, &GreedyPaintPagePaint::onAnchorChanged);
    connect(_model, &GreedyPaintConfigModel::stopTimeChanged,
        this, &GreedyPaintPagePaint::paintTmpTrain);
}

void GreedyPaintPagePaint::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;

    edTrainName=new QLineEdit;

    gpDir = new RadioButtonGroup<2>({ "下行","上行" }, this);
    gpDir->insertStretch(0, 1);
    gpDir->insertWidget(0, edTrainName);
    flay->addRow(tr("车次"), gpDir);

    auto* hlay=new QHBoxLayout;
    edAnchorTime=new QTimeEdit;
    edAnchorTime->setWrapping(true);
    edAnchorTime->setDisplayFormat("hh:mm:ss");
    hlay->addWidget(edAnchorTime);
    gpAnchorRole = new RadioButtonGroup<2>({ "作为到达时刻","作为出发时刻" }, this);
    gpAnchorRole->get(0)->setChecked(true);
    hlay->addStretch(1);
    hlay->addLayout(gpAnchorRole);
    flay->addRow(tr("锚点时刻"),hlay);
    connect(edAnchorTime, &QTimeEdit::timeChanged, this, &GreedyPaintPagePaint::paintTmpTrain);
    connect(gpAnchorRole->get(0), &QRadioButton::toggled, this, &GreedyPaintPagePaint::paintTmpTrain);

    ckStarting = new QCheckBox(tr("在本段运行线始发"));
    ckTerminal = new QCheckBox(tr("在本段运行线终到"));
    ckInstaneous = new QCheckBox(tr("即时模式 (较慢)"));
    ckInstaneous->setChecked(true);
    ckTop = new QCheckBox(tr("窗口置顶"));

    connect(ckStarting, &QCheckBox::toggled, this, &GreedyPaintPagePaint::paintTmpTrain);
    connect(ckTerminal, &QCheckBox::toggled, this, &GreedyPaintPagePaint::paintTmpTrain);


    hlay = new QHBoxLayout;
    hlay->addWidget(ckStarting);
    hlay->addWidget(ckTerminal);
    hlay->addWidget(ckInstaneous);
    hlay->addWidget(ckTop);
    connect(ckTop, &QCheckBox::toggled, this, &GreedyPaintPagePaint::setTopLevel);
    flay->addRow(tr("选项"), hlay);

    hlay = new QHBoxLayout;
    edStart = new QLineEdit;
    edStart->setReadOnly(true);
    edAnchor = new QLineEdit;
    edAnchor->setReadOnly(true);
    edEnd = new QLineEdit;
    edEnd->setReadOnly(true);

    hlay->addWidget(new QLabel(tr("起始站")));
    hlay->addWidget(edStart);
    hlay->addStretch(1);
    hlay->addWidget(new QLabel("->"));
    hlay->addStretch(1);
    hlay->addWidget(new QLabel(tr("锚点站")));
    hlay->addWidget(edAnchor);
    hlay->addStretch(1);
    hlay->addWidget(new QLabel("->"));
    hlay->addStretch(1);
    hlay->addWidget(new QLabel(tr("终止站")));
    hlay->addWidget(edEnd);
    flay->addRow(tr("铺画范围"), hlay);

    vlay->addLayout(flay);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(_model);
    table->setEditTriggers(QTableView::AllEditTriggers);
    table->setItemDelegateForColumn(GreedyPaintConfigModel::ColMinute,
        new PostiveSpinDelegate(1, this));
    table->setItemDelegateForColumn(GreedyPaintConfigModel::ColSecond,
        new SecondSpinDelegate(this));
    auto* tmdele = new QETimeDelegate(this);
    table->setItemDelegateForColumn(GreedyPaintConfigModel::ColArrive,tmdele);
    table->setItemDelegateForColumn(GreedyPaintConfigModel::ColDepart,tmdele);
    table->setItemDelegateForColumn(GreedyPaintConfigModel::ColActualStop,
        new TimeIntervalDelegate(this));
    vlay->addWidget(table);

    int c = 0;
    for (auto w : { 120,40,40,40,60,60,70,80,80 }) {
        table->setColumnWidth(c++, w);
    }

    gpDir->get(0)->setChecked(true);
    connect(gpDir->get(0),&QRadioButton::toggled,
            this,&GreedyPaintPagePaint::onDirChanged);

    txtOut = new QTextBrowser(this);
    txtOut->setWindowFlag(Qt::Dialog);
    txtOut->resize(600, 600);
    txtOut->setWindowTitle(tr("排图报告"));

    hlay=new QHBoxLayout;
    auto* btn=new QPushButton(tr("提交"));
    hlay->addStretch(1);
    hlay->addWidget(btn);
    connect(btn,&QPushButton::clicked,this,&GreedyPaintPagePaint::onApply);
    btn = new QPushButton(tr("报告"));
    hlay->addWidget(btn);
    connect(btn, &QPushButton::clicked, txtOut, &QWidget::show);
    btn=new QPushButton(tr("关闭"));
    connect(btn,&QPushButton::clicked,this,&GreedyPaintPagePaint::actClose);
    btn = new QPushButton(tr("清理"));
    connect(btn, &QPushButton::clicked, this, &GreedyPaintPagePaint::onClearTmp);
    btn->setToolTip(tr("清除当前未提交的临时列车运行线"));
    hlay->addWidget(btn);
    //btn = new QPushButton(tr("退出"));
    //btn->setToolTip(tr("退出贪心排图模式并关闭（删除）当前窗口"));
    //connect(btn, &QPushButton::clicked, this, &GreedyPaintPagePaint::actExit);
    //hlay->addWidget(btn);
    //hlay->addWidget(btn);

    vlay->addLayout(hlay);
}

std::shared_ptr<Train> GreedyPaintPagePaint::doPaintTrain()
{
    TrainName tn(edTrainName->text());
    if (!diagram.trainCollection().trainNameIsValid(tn, nullptr)) {
        QMessageBox::warning(this, tr("错误"), 
            tr("非法车次：请输入一个非空且不重复的车次。\n" 
                "注：如果当前车次已经提交排图，请撤销后再重新排图。"));
        return nullptr;
    }

    painter.setDir(DirFunc::fromIsDown(gpDir->get(0)->isChecked()));
    painter.setAnchorTime(edAnchorTime->time());
    painter.setLocalStarting(ckStarting->isChecked());
    painter.setLocalTerminal(ckTerminal->isChecked());

    painter.setAnchor(_model->anchorStation());
    painter.setStart(_model->startStation());
    painter.setEnd(_model->endStation());
    painter.setAnchorAsArrive(gpAnchorRole->get(0)->isChecked());
    painter.settledStops() = _model->stopSeconds();

    using namespace std::chrono_literals;
    auto tm_start = std::chrono::system_clock::now();
    bool res = painter.paint(tn);
    auto tm_end = std::chrono::system_clock::now();
    emit showStatus(tr("自动推线 用时 %1 毫秒").arg((tm_end - tm_start) / 1ms));

    // 整理报告
    QString report;
    report.append(tr("已配置间隔约束：\n%1\n").arg(painter.constraints().toString()));
    report.append(tr("\n--------------------------------------\n运行记录：\n"));

    int i = 0;
    for (const auto& t : painter.logs()) {
        report.append(tr("%1. %2\n").arg(++i).arg(t->toString()));
    }

    txtOut->setText(report);

    if (res) {
        //QMessageBox::information(this, tr("提示"), tr("排图成功"));
    }
    else {
        QMessageBox::warning(this, tr("提示"), tr("排图失败，可能因为没有满足约束条件的线位。"
            "已保留最后尝试状态。"));
    }

    return painter.train();
}

void GreedyPaintPagePaint::resetTmpTrain()
{
    if (trainRef) {
        trainTmp = std::make_shared<Train>(*trainRef);
    }
    else {
        trainTmp = std::make_shared<Train>(painter.train()->trainName());
        trainTmp->setType(diagram.trainCollection().typeManager().fromRegex(trainTmp->trainName()));
    }
}

void GreedyPaintPagePaint::mergeTmpTrain()
{
    resetTmpTrain();

    if (true) {
        // only new train for now
        trainTmp->timetable() = std::move(painter.train()->timetable());
    }

    if (painter.localStarting()) {
        trainTmp->setStarting(painter.train()->starting());
    }
    if (painter.localTerminal()) {
        trainTmp->setTerminal(painter.train()->terminal());
    }
}

void GreedyPaintPagePaint::onDirChanged(bool down)
{
    _model->setDir(DirFunc::fromIsDown(down));
    _model->refreshData();
}

void GreedyPaintPagePaint::paintTmpTrain()
{
    if (!ckInstaneous->isChecked())
        return;
    if (!painter.ruler()) {
        // 终止条件，或许不止这个
        QMessageBox::warning(this, tr("错误"), tr("无效标尺！"));
        return;
    }

    if (_model->startRow() == _model->endRow()) {
        // 空车次拒绝铺画
        return;
    }

    if (trainTmp) {
        emit removeTmpTrainLine(*trainTmp);
    }

    auto train = doPaintTrain();
    if (!train)return;

    _model->setTimetable(train);

    mergeTmpTrain();

    diagram.updateTrain(trainTmp);
    emit paintTmpTrainLine(*trainTmp);

}

void GreedyPaintPagePaint::onApply()
{
    if (!painter.ruler()) {
        // 终止条件，或许不止这个
        QMessageBox::warning(this, tr("错误"), tr("无效标尺！"));
        return;
    }

    if (trainTmp) {
        emit removeTmpTrainLine(*trainTmp);
    }

    if (_model->startStation() != _model->anchorStation()) {
        QMessageBox::information(this, tr("提示"),
            tr("此预览版暂未实现反推运行线功能，锚点站将被作为实际的起始站；"
                "设置的起始站至锚点站之间的运行线不会被计算。"));
    }

    auto train = doPaintTrain();
    if (!train)return;

    _model->setTimetable(train);

    mergeTmpTrain();

    // 采用新的对象来添加，因为当前窗口现在不会被立即销毁
    auto newtrain = std::make_shared<Train>(std::move(*trainTmp));
    trainTmp.reset();

    diagram.updateTrain(newtrain);
    emit trainAdded(newtrain);

}

void GreedyPaintPagePaint::onStartChanged(std::shared_ptr<const RailStation> st)
{
    edStart->setText(st->name.toSingleLiteral());
    paintTmpTrain();
}

void GreedyPaintPagePaint::onEndChanged(std::shared_ptr<const RailStation> st)
{
    edEnd->setText(st->name.toSingleLiteral());
    paintTmpTrain();
}

void GreedyPaintPagePaint::onAnchorChanged(std::shared_ptr<const RailStation> st)
{
    edAnchor->setText(st->name.toSingleLiteral());
    paintTmpTrain();
}

void GreedyPaintPagePaint::onClearTmp()
{
    if (trainTmp) {
        emit removeTmpTrainLine(*trainTmp);
    }
}

void GreedyPaintPagePaint::setTopLevel(bool on)
{
    nativeParentWidget()->setWindowFlag(Qt::WindowStaysOnTopHint, on);
    if (on) {
        nativeParentWidget()->show();
    }
}

void GreedyPaintPagePaint::setupStationLabels()
{
    edStart->setText(_model->startStation()->name.toSingleLiteral());
    edEnd->setText(_model->endStation()->name.toSingleLiteral());
    edAnchor->setText(_model->anchorStation()->name.toSingleLiteral());
}
