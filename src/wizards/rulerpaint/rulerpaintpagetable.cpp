#include "rulerpaintpagetable.h"
#include "rulerpaintpagestation.h"


#include <QComboBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QAction>
#include <QHeaderView>
#include <QMessageBox>
#include "data/rail/rail.h"
#include "data/diagram/diagram.h"
#include "data/common/qesystem.h"
#include "rulerpaintwizard.h"
#include "model/delegate/qedelegate.h"
#include "model/delegate/generalspindelegate.h"
#include "model/delegate/postivespindelegate.h"
#include "model/delegate/traintimedelegate.h"
#include "util/utilfunc.h"
#include "viewers/traintimetableplane.h"
#include "dialogs/selecttrainstationdialog.h"
#include "dialogs/batchaddstopdialog.h"
#include "util/traintimeedit.h"


RulerPaintModel::RulerPaintModel(RulerPaintPageTable *page_, QObject *parent):
    QStandardItemModel(parent),page(page_),cbStart(new QComboBox),cbEnd(new QComboBox)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
    tr("站名"),tr("停分"),tr("停秒"),tr("到点"),tr("开点"),tr("附加"),tr("调整"),
    tr("区间")
                              });
    connect(cbStart,SIGNAL(currentIndexChanged(int)),this,
            SLOT(onStartChanged(int)));
    connect(cbEnd,SIGNAL(currentIndexChanged(int)),this,
            SLOT(onEndChanged(int)));
    connect(this,
        SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)),
        this, SLOT(onDataChanged(const QModelIndex&, const QModelIndex&,
            const QVector<int>&)));
}

void RulerPaintModel::setupModel(std::shared_ptr<Railway> railway_,
                                 std::shared_ptr<Ruler> ruler_,
                                 Direction dir_,
                                 std::shared_ptr<const RailStation> anchorStation_)
{
    railway=railway_;
    ruler=ruler_;
    dir=dir_;
    anchorStation=anchorStation_;

    beginResetModel();
    updating=true;
    setRowCount(0);

    //先向前找能够插入的
    cbStart->clear();
    std::shared_ptr<const RulerNode> node{};
    //注意不能用dirAdjacent往回找，因为要找的不是反方向近邻，而是本方向的前序近邻
    for(auto st=anchorStation;st; st = node->railInterval().fromStation()){
        insertRow(0);
        initRow(0, st, node);
        cbStart->addItem(st->name.toSingleLiteral());   // 暂定倒序
        node=ruler->dirPrevNode(st,dir);
        if (!node || node->isNull())
            break;
    }

    int row=rowCount();    //这是 将要插入的行
    _anchorRow=row-1;

    QColor color=Qt::blue;
    color.setAlpha(55);
    for(int c=0;c<ColMAX;c++){
        item(row-1,c)->setBackground(color);
    }

    //现在开始处理后面的
    cbEnd->clear();
    cbEnd->addItem(anchorStation->name.toSingleLiteral());
    for(auto st=anchorStation->dirAdjacent(dir);st;st=st->dirAdjacent(dir)){
        auto n=ruler->dirPrevNode(st,dir);
        if(!n||n->isNull())
            break;
        //现在 当前站有效且没有插入过
        cbEnd->addItem(st->name.toSingleLiteral());
        insertRow(row);
        initRow(row++, st, n);
    }
    updateFromRow(_anchorRow);

    startRow = endRow = _anchorRow;
    endResetModel();
    updating=false;
}

void RulerPaintModel::setAnchorStopSecs(int secs)
{
    item(_anchorRow,ColMinute)->setData(secs/60,Qt::EditRole);
    item(_anchorRow,ColSecond)->setData(secs%60,Qt::EditRole);
}

std::shared_ptr<const RailStation> RulerPaintModel::getRowStation(int row) const
{
    return qvariant_cast<std::shared_ptr<const RailStation>> (
        item(row, ColStation)->data(qeutil::RailStationRole));
}

int RulerPaintModel::getStationRow(std::shared_ptr<const RailStation> st) const
{
    for (int i = 0; i < rowCount(); i++) {
        auto sti = getRowStation(i);
        if (st == sti)
            return i;
    }
    return -1;
}

TrainTime RulerPaintModel::getRowArrive(int row) const
{
    return TrainTime::fromQVariant(item(row, ColArrive)->data(Qt::EditRole));
}

TrainTime RulerPaintModel::getRowDepart(int row) const
{
    return TrainTime::fromQVariant(item(row, ColDepart)->data(Qt::EditRole));
}

void RulerPaintModel::setStationStopSecs(const StationName& name, int secs)
{
    for (int i = 0; i < rowCount(); i++) {
        const auto& name_s = item(i, ColStation)->data(Qt::DisplayRole);
        if (name.equalOrContains(name_s.toString())) {
            setStopSecs(i, secs);
            break;
        }
    }
}

void RulerPaintModel::initRow(int row, std::shared_ptr<const RailStation> st,
    std::shared_ptr<const RulerNode> node)
{
    using SI=QStandardItem;
    auto* it=new SI(st->name.toSingleLiteral());
    it->setEditable(false);
    QVariant v;
    v.setValue(st);
    it->setData(v, qeutil::RailStationRole);
    setItem(row,ColStation,it);

    it=new SI;
    it->setData(0,Qt::EditRole);
    setItem(row,ColMinute,it);

    it=new SI;
    it->setData(0,Qt::EditRole);
    setItem(row,ColSecond,it);

    it=new SI;
    it->setData(page->anchorTime().toQVariant(), Qt::EditRole);   //时刻一律初始化为参考时刻
    it->setEditable(false);
    setItem(row,ColArrive,it);

    it=new SI;
    it->setData(page->anchorTime().toQVariant(), Qt::EditRole);
    it->setEditable(false);
    setItem(row,ColDepart,it);

    if (st == anchorStation) {
        it = new SI(tr("--"));
        it->setEnabled(false);
    }
    else
        it=new SI(tr("通通"));
    it->setEditable(false);
    setItem(row,ColAppend,it);

    it=new SI;
    it->setData(0,Qt::EditRole);
    if(st==anchorStation){
        it->setEnabled(false);
    }
    setItem(row,ColAdjust,it);

    //暂定不保存数据，只用来显示
    it=new SI("--");
    it->setEditable(false);
    if(st==anchorStation){
        it->setEnabled(false);
    }
    v.setValue(node);
    it->setData(v, qeutil::RulerNodeRole);
    setItem(row,ColInterval,it);
}

void RulerPaintModel::updateFromRow(int row)
{
    if (row >= _anchorRow) {
        //向后调整
    }
    else {
        //向前调整
    }

    //下面才是正式的代码
    int dr = (row >= _anchorRow ? 1 : -1);
    int r = row;
    
    if (row == _anchorRow) {
        //如果起始站为anchor，假定到达时刻正确，调整出发时刻
        const TrainTime& arr = TrainTime::fromQVariant(item(row, ColArrive)->data(Qt::EditRole));
        item(row, ColDepart)->setData(arr.addSecs(getStopSecs(r), periodHours()).toQVariant(), Qt::EditRole);
    }
    else {
        // 起始行非参考行，设置interval情况；当前的时刻要由上一区间来计算
        //注意此时Prev行一定存在！
        int pr = r - dr;
        bool cu_stopped = bool(getStopSecs(r));
        bool pr_stopped = bool(getStopSecs(pr));
        TrainTime tm = TrainTime::fromQVariant(dr == 1 ?
            item(pr, ColDepart)->data(Qt::EditRole) :
            item(pr, ColArrive)->data(Qt::EditRole)
            );

        calRowTime(r, dr, pr_stopped, cu_stopped, tm);
    }

    //循环准备
    int prev = r;
    bool prev_stopped = bool(getStopSecs(r));
    bool cur_stopped;
    
    //循环的上一站的那个时刻
    TrainTime prev_time = TrainTime::fromQVariant(dr == 1 ?
        item(r, ColDepart)->data(Qt::EditRole) :
        item(r, ColArrive)->data(Qt::EditRole));

    //loop invariant: r是当前要处理的行，prev是上一个依赖的行；
    //循环中一次性处理好一行的信息
    //注意记录下来当前站、后一站是否停车等信息
    //注意r不会是anchor那一行
    for (r += dr; r >= 0 && r < rowCount(); prev = r, r += dr) {
        cur_stopped = bool(getStopSecs(r));
        TrainTime tm = prev_time;
        calRowTime(r, dr, prev_stopped, cur_stopped, tm);

        //循环结束操作：
        prev_stopped = cur_stopped;
        prev_time = tm;
    }

    if (row > 0 && row == _anchorRow) {
        //对于要直接处理anchor变化的情况，前面那部分交给递归调用
        updateFromRow(row - 1);
    }
}

int RulerPaintModel::getStopSecs(int row) const
{
    return item(row,ColMinute)->data(Qt::EditRole).toInt() * 60 +
            item(row,ColSecond)->data(Qt::EditRole).toInt();
}

TrainTime RulerPaintModel::getArrive(int row) const
{
    return TrainTime::fromQVariant(item(row, ColArrive)->data(Qt::EditRole));
}

TrainTime RulerPaintModel::getDepart(int row) const
{
    return TrainTime::fromQVariant(item(row, ColDepart)->data(Qt::EditRole));
}

void RulerPaintModel::setStopSecs(int row, int secs)
{
    int m = secs / 60, s = secs % 60;
    item(row, ColMinute)->setData(m, Qt::EditRole);
    item(row, ColSecond)->setData(s, Qt::EditRole);
}



int RulerPaintModel::calRowAppendInterval(int r, int dr, bool prev_stopped, bool cur_stopped)
{
    auto node = qvariant_cast<std::shared_ptr<const RulerNode>>(
        item(r, ColInterval)->data(qeutil::RulerNodeRole));
    int interval = node->interval;
    interval += item(r, ColAdjust)->data(Qt::EditRole).toInt();
    QString append;

    // 新版：直接分方向讨论
    if (dr == 1) {
        //正推
        if (prev_stopped) {
            interval += node->start;
            append = tr("起");
        }
        else {
            if (startRow == r - dr && page->startAtThis()) {
                //始发
                interval += node->start;
                append = tr("始");
            }
            else {
                append = tr("通");
            }
        }
        if (cur_stopped) {
            interval += node->stop;
            append.append(tr("停"));
        }
        else {
            if (endRow == r && page->endAtThis()) {
                //终到
                interval += node->stop;
                append.append(tr("终"));
            }
            else {
                append.append(tr("通"));
            }
        }
    }
    else {
        //反推
        if (cur_stopped) {
            interval += node->start;
            append = tr("起");
        }
        else {
            if (startRow == r && page->startAtThis()) {
                interval += node->start;
                append = tr("始");
            }
            else {
                append = tr("通");
            }
        }
        if (prev_stopped) {
            interval += node->stop;
            append.append(tr("停"));
        }
        else {
            if (r - dr == endRow && page->endAtThis()) {
                interval += node->stop;
                append.append(tr("终"));
            }
            else {
                append.append("通");
            }
        }
    }

    item(r, ColAppend)->setText(append);
    return interval;
}

bool RulerPaintModel::calRowTime(int r, int dr, bool prev_stopped, bool cur_stopped, TrainTime& tm0)
{
    int interval = calRowAppendInterval(r, dr, prev_stopped, cur_stopped);
    item(r, ColInterval)->setText(qeutil::secsDiffToString(interval));

    //下面：推时刻  1,2 分别表示接近和远离anchor的那个时刻
    TrainTime tm = tm0.addSecs(dr * interval, page->getDiagram().options().period_hours);  //正推的到达时刻，倒推的出发时刻
    bool flag1, flag2;   //如果到达、出发时刻都没变，就可以提前结束了

    int col1 = (dr == 1 ? ColArrive : ColDepart);
    int col2 = (dr == 1 ? ColDepart : ColArrive);

    flag1 = (TrainTime::fromQVariant( item(r, col1)->data(Qt::EditRole)) == tm);
    if (!flag1) {
        item(r, col1)->setData(tm.toQVariant(), Qt::EditRole);
    }
    tm = tm.addSecs(dr * getStopSecs(r), page->getDiagram().options().period_hours);
    flag2 = (TrainTime::fromQVariant(item(r, col2)->data(Qt::EditRole)) == tm);
    if (!flag2) {
        item(r, col2)->setData(tm.toQVariant(), Qt::EditRole);
    }
    tm0 = tm;

    return flag1 && flag2;
}

std::shared_ptr<Train> RulerPaintModel::toTrain() const
{
    auto t=std::make_shared<Train>(tr("铺画车次"));
    for(int r=startRow;r<=endRow;r++){
        t->appendStation(
            StationName::fromSingleLiteral(item(r, ColStation)->text()),
            TrainTime::fromQVariant(item(r, ColArrive)->data(Qt::EditRole)),
            TrainTime::fromQVariant(item(r, ColDepart)->data(Qt::EditRole))
        );
    }
    if (page->startAtThis()) {
        t->setStarting(StationName::fromSingleLiteral(item(startRow, ColStation)->text()));
    }
    if (page->endAtThis()) {
        t->setTerminal(StationName::fromSingleLiteral(item(endRow, ColStation)->text()));
    }
    return t;
}

int RulerPaintModel::periodHours() const
{
    return page->getDiagram().options().period_hours;
}


// 注意这个入参i是指combo的下标
void RulerPaintModel::onStartChanged(int i)
{
    if (updating)return;
    int r = std::max(startRow, _anchorRow - i);
    startRow = _anchorRow - i;
    updateFromRow(r);
    if (page->instaneous())
        paintTrain();
}

void RulerPaintModel::onEndChanged(int i)
{
    if (updating)return;
    int r = std::min(endRow, _anchorRow + i);
    endRow = _anchorRow + i;
    updateFromRow(r);
    if (page->instaneous())
        paintTrain();
}

void RulerPaintModel::onAnchorTimeChanged(const TrainTime &tm)
{
    if(page->anchorAsArrive()){
        item(_anchorRow,ColArrive)->setData(tm.toQVariant(), Qt::EditRole);
    }else{
        item(_anchorRow, ColArrive)->setData(
            tm.addSecs(-getStopSecs(_anchorRow), page->getDiagram().options().period_hours).toQVariant(),
            Qt::EditRole);
    }
    updateFromRow(_anchorRow);
    if(page->instaneous())
        paintTrain();
}

void RulerPaintModel::onAnchorTypeChanged()
{
    onAnchorTimeChanged(page->anchorTime());
}

void RulerPaintModel::paintTrain()
{
    if (startRow < endRow) {
        auto train = toTrain();
        emit updateTrainLine(train);
    }
}

void RulerPaintModel::onStartAtThisChanged()
{
    updateFromRow(_anchorRow);
    if (page->instaneous())
        paintTrain();
}

void RulerPaintModel::onEndAtThisChanged()
{
    updateFromRow(_anchorRow);
    if (page->instaneous())
        paintTrain();
}

void RulerPaintModel::onRowEditDataChanged(int row)
{
    // 2025.07.12: For start or end row changed, we always update from the anchor row (i.e., update all data).
    // The calculation should not be very slow, according to our observations...
    bool startOrEndChanged = false;
    if (row <= _anchorRow && (row < startRow || startRow == -1)) {
        startOrEndChanged = true;
        setStartRow(row);
    }
    if (row >= _anchorRow && row > endRow) {
        startOrEndChanged = true;
        setEndRow(row);
    }
    updateFromRow(startOrEndChanged ? _anchorRow : row);
    if (page->instaneous()) {
        paintTrain();
    }
}

void RulerPaintModel::setStartRow(int r)
{
    updating = true;
    startRow = r;
    cbStart->setCurrentIndex(_anchorRow - r);
    updating = false;
}

void RulerPaintModel::setEndRow(int r)
{
    updating = true;
    endRow = r;
    cbEnd->setCurrentIndex(r - _anchorRow);
    updating = false;
}

void RulerPaintModel::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    if (roles.contains(Qt::EditRole) || roles.contains(Qt::DisplayRole)) {
        if (std::max(topLeft.column(), (int)ColMinute) <=
            std::min(bottomRight.column(), (int)ColSecond) ||
            (topLeft.column() <= ColAdjust && bottomRight.column() >= ColAdjust)) {
            //此条件为：指定的三列之一被包括
            for (int r = topLeft.row(); r <= bottomRight.row(); r++) {
                onRowEditDataChanged(r);
            }
        }
    }
}

RulerPaintPageTable::RulerPaintPageTable(Diagram& diagram_, 
    RulerPaintPageStation* pgStation_, QWidget *parent):
    QWizardPage(parent),diagram(diagram_),
    pgStation(pgStation_),conflictDialog(new ConflictDialog(diagram_,this)),
    model(new RulerPaintModel(this,this))
{
    initUI();
}

void RulerPaintPageTable::initializePage()
{
    railway=pgStation->railway();
    ruler=pgStation->ruler();
    auto dir=pgStation->getDir();
    auto st=pgStation->anchorStation();

    model->setupModel(railway,ruler,dir,st);
    table->resizeColumnsToContents();

    anchorRow=model->anchorRow();

    edAnName->setText(st->name.toSingleLiteral());

    setSubTitle(tr("现在使用[%1]标尺在[%2]线路上，以[%3]站为锚点，按[%4]方向排图。"
        "双击排图到指定行，按Alt+X进行冲突检查。").arg(ruler->name()).arg(railway->name())
                .arg(st->name.toSingleLiteral()) .arg(DirFunc::dirToString(dir)));
}

bool RulerPaintPageTable::startAtThis() const
{
    return gpChecks->get(0)->isChecked();
}

bool RulerPaintPageTable::endAtThis() const
{
    return gpChecks->get(1)->isChecked();
}

bool RulerPaintPageTable::instaneous() const
{
    return gpChecks->get(2)->isChecked();
}

TrainTime RulerPaintPageTable::anchorTime() const
{
    return edAnTime->time();
}

void RulerPaintPageTable::setAnchorTime(const TrainTime &arr, const TrainTime &dep)
{
    int stopsec = qeutil::secsTo(arr, dep, diagram.options().period_hours);
    if(anchorAsArrive()){
        edAnTime->setTime(arr);
    }else{
        edAnTime->setTime(dep);
    }

    if(!model->rowCount())
        return;
    model->setAnchorStopSecs(stopsec);
}

void RulerPaintPageTable::setRefTrain(std::shared_ptr<Train> train)
{
    timetableRef->setTrain(train);
}

void RulerPaintPageTable::initUI()
{
    setTitle(tr("排图"));
    //subtitle留给概要
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;

    auto* hlay=new QHBoxLayout;
    edAnTime=new TrainTimeEdit;
    edAnTime->setFormat(TrainTime::HMS);
    edAnTime->setMaxHours(diagram.options().period_hours);
    connect(edAnTime, &TrainTimeEdit::timeChanged,
            model,&RulerPaintModel::onAnchorTimeChanged);
    hlay->addWidget(edAnTime);
    hlay->addStretch(1);
    gpAnType=new RadioButtonGroup<2>({"作为到达时刻","作为出发时刻"},this);
    hlay->addStretch(1);
    gpAnType->get(0)->setChecked(true);
    hlay->addLayout(gpAnType);
    gpAnType->connectAllTo(SIGNAL(toggled(bool)),model,SLOT(onAnchorTypeChanged()));

    auto* btn=new QPushButton(tr("导入停站"));
    hlay->addStretch(2);
    hlay->addWidget(btn);
    connect(btn,&QPushButton::clicked,
            this,&RulerPaintPageTable::loadStopTime);
    btn = new QPushButton(tr("批量添加停点"));
    hlay->addWidget(btn);
    connect(btn, &QPushButton::clicked,
        this, &RulerPaintPageTable::batchAddStop);

    flay->addRow(tr("锚点时刻"),hlay);

    gpChecks=new ButtonGroup<3,QHBoxLayout,QCheckBox>({"在本段运行线始发","在本段运行线终到","即时铺画"});
    gpChecks->get(2)->setChecked(true);
    flay->addRow(tr("选项"),gpChecks);

    connect(gpChecks->get(0), SIGNAL(toggled(bool)), model, SLOT(onStartAtThisChanged()));
    connect(gpChecks->get(1), SIGNAL(toggled(bool)), model, SLOT(onEndAtThisChanged()));

    hlay=new QHBoxLayout;
    cbStart=model->getComboStart();
    cbEnd=model->getComboEnd();
    hlay->addWidget(new QLabel("起始站:"));
    hlay->addWidget(cbStart);
    hlay->addStretch(1);
    hlay->addWidget(new QLabel("->"));
    hlay->addStretch(1);

    edAnName=new QLineEdit;
    edAnName->setFocusPolicy(Qt::NoFocus);
    edAnName->setWhatsThis(tr("锚点站站名\n仅作提示，不可编辑。如需更换，请回到上一步操作"));
    hlay->addWidget(new QLabel("锚点站:"));
    hlay->addWidget(edAnName);

    hlay->addStretch(1);
    hlay->addWidget(new QLabel("->"));
    hlay->addStretch(1);

    hlay->addWidget(new QLabel("终止站:"));
    hlay->addWidget(cbEnd);
    flay->addRow(tr("铺画范围"),hlay);


    vlay->addLayout(flay);

    table=new QTableView;
    table->setEditTriggers(QTableView::AllEditTriggers);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    connect(table, &QTableView::doubleClicked,this,
            &RulerPaintPageTable::onDoubleClicked);

    table->setItemDelegateForColumn(RulerPaintModel::ColMinute,
        new PostiveSpinDelegate(1, this));
    table->setItemDelegateForColumn(RulerPaintModel::ColSecond,
        new PostiveSpinDelegate(10, this));
    table->setItemDelegateForColumn(RulerPaintModel::ColAdjust,
        new SteppedSpinDelegate(10, this));
    auto* dele = new TrainTimeDelegate(diagram.options(), this);
    table->setItemDelegateForColumn(RulerPaintModel::ColArrive, dele);
    table->setItemDelegateForColumn(RulerPaintModel::ColDepart, dele);

    // context menu
    table->setContextMenuPolicy(Qt::ActionsContextMenu);

    auto* act = new QAction(tr("冲突检查"), table);
    act->setShortcut(Qt::ALT | Qt::Key_X);
    table->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(showConflict()));

    act = new QAction(tr("查看参考时刻表"), table);
    act->setShortcut(Qt::ALT | Qt::Key_Y);
    table->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(showTimetable()));

    vlay->addWidget(table);

    // 时刻表dialog
    auto* t = new TrainTimetablePlane(diagram.options());
    timetableRef = t;
    t->resize(500, 500);
    timeDialog = new DialogAdapter(t, this);
    connect(timetableRef, &QWidget::windowTitleChanged,
        timeDialog, &QWidget::setWindowTitle);
    timeDialog->setAttribute(Qt::WA_DeleteOnClose, false);
}

void RulerPaintPageTable::showConflict()
{
    auto idx = table->currentIndex();
    if (!idx.isValid())return;
    conflictDialog->setData(railway, model->getRowStation(idx.row()),
        model->getRowArrive(idx.row()), model->getRowDepart(idx.row()));
    if (!conflictDialog->isVisible())
        conflictDialog->show();
}

void RulerPaintPageTable::showTimetable()
{
    if (!timeDialog->isVisible())
        timeDialog->show();
}

void RulerPaintPageTable::loadStopTime()
{
    auto p = QMessageBox::question(this, tr("导入停站"),
        tr("选择车次及其时刻表部分站点，将其停站时长导入当前铺画表格中。\n"
            "车站按站名匹配，重复站名以后出现的为准，且将覆盖当前设置。\n"
            "是否确认？"));
    if (p != QMessageBox::Yes)
        return;
    auto res=SelectTrainStationsDialog::dlgGetStation(diagram.trainCollection(), diagram.options(),
                                                      this);
    for (const auto& t : res) {
        model->setStationStopSecs(t->name, t->stopSec(diagram.options().period_hours));
    }
}

void RulerPaintPageTable::batchAddStop()
{
    auto rep = BatchAddStopDialog::setBatchAdd(this);
    if (!rep.has_value())
        return;

    if (rep->constr_range) {
        auto* selmod = table->selectionModel();
        auto sel = selmod->selectedRows();
        foreach(const auto & idx, sel) {
            auto st = model->getRowStation(idx.row());
            if (!rep->constr_level || st->level <= rep->lowest_level)
                model->setStopSecs(idx.row(), rep->stop_secs);
        }
    }
    else {
        for (int i = 0; i < model->rowCount(); i++) {
            auto st = model->getRowStation(i);
            if (!rep->constr_level || st->level <= rep->lowest_level)
                model->setStopSecs(i, rep->stop_secs);
        }
    }
}

void RulerPaintPageTable::onDoubleClicked(const QModelIndex &idx)
{
    if(!idx.isValid())
        return;
    int r=idx.row();
    if(r==anchorRow){
        cbStart->setCurrentIndex(0);
        cbEnd->setCurrentIndex(0);
    }else if(r<anchorRow){
        //变更起始站
        cbStart->setCurrentIndex(anchorRow-r);
    }else{
        cbEnd->setCurrentIndex(r-anchorRow);
    }
    model->paintTrain();
}

