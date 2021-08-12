#include "rulerpaintpagetable.h"
#include "rulerpaintpagestation.h"
#include <QtWidgets>
#include <QTime>
#include "data/rail/rail.h"
#include "data/diagram/diagram.h"
#include "rulerpaintwizard.h"
#include "model/delegate/qedelegate.h"

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
}

void RulerPaintModel::setupModel(std::shared_ptr<Railway> railway_,
                                 std::shared_ptr<Ruler> ruler_,
                                 Direction dir_,
                                 std::shared_ptr<const RailStation> anchorStation_)
{
    using SI=QStandardItem;
    railway=railway_;
    ruler=ruler_;
    dir=dir_;
    anchorStation=anchorStation_;

    beginResetModel();
    updating=true;
    setRowCount(0);

    //先向前找能够插入的
    cbStart->clear();
    for(auto st=anchorStation;st;st=st->dirAdjacent(DirFunc::reverse(dir))){
        insertRow(0);
        initRow(0, st);
        cbStart->addItem(st->name.toSingleLiteral());   // 暂定倒序
        auto n=ruler->dirPrevNode(st,dir);
        if(!n||n->isNull())
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
        initRow(row++,st);
    }
    updateFromRow(_anchorRow);

    endResetModel();
    updating=false;
}

void RulerPaintModel::initRow(int row, std::shared_ptr<const RailStation> st)
{
    using SI=QStandardItem;
    auto* it=new SI(st->name.toSingleLiteral());
    it->setEditable(false);
    setItem(row,ColStation,it);

    it=new SI;
    it->setData(0,Qt::EditRole);
    setItem(row,ColMinute,it);

    it=new SI;
    it->setData(0,Qt::EditRole);
    setItem(row,ColSecond,it);

    it=new SI;
    it->setData(QTime(0,0),Qt::EditRole);
    it->setEditable(false);
    setItem(row,ColArrive,it);

    it=new SI;
    it->setData(QTime(0,0),Qt::EditRole);
    it->setEditable(false);
    setItem(row,ColDepart,it);

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
    setItem(row,ColInterval,it);
}

void RulerPaintModel::updateFromRow(int row)
{
    //todo
}

int RulerPaintModel::getStopSecs(int row) const
{
    return item(row,ColMinute)->data(Qt::EditRole).toInt() * 60 +
            item(row,ColSecond)->data(Qt::EditRole).toInt();
}

void RulerPaintModel::onStartChanged(int i)
{
    // todo
}

void RulerPaintModel::onEndChanged(int i)
{
    // todo
}

void RulerPaintModel::onAnchorTimeChanged(const QTime &tm)
{
    if(page->anchorAsArrive()){
        item(_anchorRow,ColArrive)->setData(tm, Qt::EditRole);
    }else{
        item(_anchorRow,ColArrive)->setData(tm.addSecs(-getStopSecs(_anchorRow)),
                                            Qt::EditRole);
    }
    updateFromRow(_anchorRow);
}

void RulerPaintModel::onAnchorTypeChanged()
{
    onAnchorTimeChanged(page->anchorTime());
}

void RulerPaintModel::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    // todo
}

RulerPaintPageTable::RulerPaintPageTable(RulerPaintPageStation* pgStation_, QWidget *parent):
    QWizardPage(parent),pgStation(pgStation_), tmptrain(std::make_shared<Train>(tr("排图临时"))),
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

QTime RulerPaintPageTable::anchorTime() const
{
    return edAnTime->time();
}

void RulerPaintPageTable::initUI()
{
    setTitle(tr("排图"));
    //subtitle留给概要
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;

    auto* hlay=new QHBoxLayout;
    edAnTime=new QTimeEdit;
    edAnTime->setDisplayFormat("hh:mm:ss");
    connect(edAnTime,&QTimeEdit::timeChanged,
            model,&RulerPaintModel::onAnchorTimeChanged);
    hlay->addWidget(edAnTime);
    hlay->addStretch(1);
    gpAnType=new RadioButtonGroup<2>({"作为到达时刻","作为出发时刻"},this);
    hlay->addStretch(1);
    gpAnType->get(0)->setChecked(true);
    hlay->addLayout(gpAnType);
    gpAnType->connectAllTo(SIGNAL(toggled(bool)),model,SLOT(onAnchorTypeChanged()));
    flay->addRow(tr("锚点时刻"),hlay);

    gpChecks=new ButtonGroup<3,QHBoxLayout,QCheckBox>({"在本线始发","在本线终到","即时铺画"});
    gpChecks->get(2)->setChecked(true);
    flay->addRow(tr("选项"),gpChecks);

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
    table->setEditTriggers(QTableView::CurrentChanged);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    connect(table, &QTableView::doubleClicked,this,
            &RulerPaintPageTable::onDoubleClicked);
    vlay->addWidget(table);
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
}
