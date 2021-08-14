#include "rulerpaintpagestart.h"
#include "util/selecttraincombo.h"
#include <QtWidgets>

#include <algorithm>
#include "data/diagram/diagram.h"
#include "util/utilfunc.h"

RulerPaintPageStart::RulerPaintPageStart(TrainCollection &coll_, QWidget *parent):
    QWizardPage(parent),coll(coll_),model(new TimetableStdModel(false,this))
{
    initUI();
}

bool RulerPaintPageStart::validatePage()
{
    auto m=getMode();
    if(m==PaintMode::NewTrain){
        //检查新车次是否合法
        if(!coll.trainNameIsValid(edName->text(),{})){
            QMessageBox::warning(this,tr("错误"),
                                 tr("铺画新车次：请输入一个非空并且不与既有冲突的全车次!"));
            return false;
        }else{
            auto t = std::make_shared<Train>(edName->text());
            t->setType(coll.typeManager().fromRegex(t->trainName()));
            _train = t;
            return true;
        }
    }
    //现在：需要读取既有车次
    auto t=sels[m-1]->train();
    if(!t){
        QMessageBox::warning(this,tr("错误"),
                             tr("请先选择一个既有车次！"));
        return false;
    }

    if(m==PaintMode::Modify){
        auto rg=table->selectionModel()->selectedRows();
        if(rg.isEmpty()){
            QMessageBox::warning(this,tr("错误"),tr("重排既有车次的部分运行线："
                "请选择需要覆盖排图的范围！"));
            return false;
        }

        _startRow=std::min_element(rg.begin(),rg.end(),&qeutil::ltIndexRow)->row();
        _endRow=std::max_element(rg.begin(),rg.end(),&qeutil::ltIndexRow)->row();
    }
    _train=t;
    return true;

}

void RulerPaintPageStart::initUI()
{
    setTitle(tr("开始"));
    setSubTitle(tr("欢迎使用标尺排图向导。此功能覆盖原pyETRC中的[标尺排图向导]和[区间重排]两个功能，"
        "取决于所选模式的不同。"));
    auto* vlay=new QVBoxLayout;
    gpMode=new RadioButtonGroup<4,QVBoxLayout>({"铺画新车次","前缀到既有车次","后缀到既有车次",
                                   "重排既有车次的部分运行线"},this);
    vlay->addLayout(gpMode);
    gpMode->get(0)->setChecked(true);
    gpMode->connectAllTo(SIGNAL(toggled(bool)),this,SLOT(onModeChanged()));

    stack=new QStackedWidget;
    stack->setFrameShape(QFrame::Box);
    initWidget0();
    initWidget1();
    initWidget2();
    initWidget3();
    vlay->addWidget(stack);

    setLayout(vlay);
}

// 铺画新车次
void RulerPaintPageStart::initWidget0()
{
    auto* w=new QWidget;
    auto* flay=new QFormLayout;

    auto* lab=new QLabel(tr("铺画新车次：所铺画的数据将作为新建的列车对象，"
        "并在铺画完毕后添加到当前运行图中。\n"
        "请输入一个有效的全车次。"));
    lab->setWordWrap(true);
    flay->addWidget(lab);
    edName=new QLineEdit;
    flay->addRow(tr("新车次"),edName);
    w->setLayout(flay);
    stack->addWidget(w);
}

//前缀到车次
void RulerPaintPageStart::initWidget1()
{
    auto* w=new QWidget;
    auto* flay=new QFormLayout;

    auto* lab=new QLabel(tr("前缀到车次：将所铺画的数据作为（新）时刻表的开头部分，"
        "更新到所选车次中。注意除非所铺画的最后一站与原有时刻表第一站为同一站的情况"
        "（此时将覆盖原时刻表第一站），否则不会比较站名，而是直接将新的时刻表插入到原时刻表最前。\n"
        "请选择一个既有车次。"));
    lab->setWordWrap(true);
    flay->addWidget(lab);

    sels[0]=new SelectTrainCombo(coll);
    flay->addRow(tr("选择车次"),sels[0]);
    w->setLayout(flay);
    stack->addWidget(w);
}

//后缀到车次
void RulerPaintPageStart::initWidget2()
{
    auto* w=new QWidget;
    auto* flay=new QFormLayout;

    auto* lab=new QLabel(tr("后缀到车次：将所铺画的数据作为（新）时刻表的结尾部分，"
        "更新到所选车次中。注意除非所铺画的第一站与原有时刻表最后一站为同一站的情况"
        "（此时将覆盖原时刻表最后一站），否则不会比较站名，而是直接将新的时刻表插入到原时刻表最后。\n"
        "请选择一个既有车次。"));
    lab->setWordWrap(true);
    flay->addWidget(lab);

    sels[1]=new SelectTrainCombo(coll);
    flay->addRow(tr("选择车次"),sels[1]);
    w->setLayout(flay);
    stack->addWidget(w);
}

//修订既有车次
void RulerPaintPageStart::initWidget3()
{
    auto* w=new QWidget;
    auto* vlay=new QVBoxLayout;
    auto* lab=new QLabel(tr("重排既有车次的部分运行线：原pyETRC中的[区间时刻重排] (Ctrl+Shift+R)功能。"
        "选择一个原有时刻表中的范围，新铺画的时刻表将覆盖这部分运行线。\n"
        "请先选择一个既有车次，然后选择要重新排图的范围。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* flay=new QFormLayout;
    sels[2]=new SelectTrainCombo(coll);
    connect(sels[2],&SelectTrainCombo::currentTrainChanged,
            this,&RulerPaintPageStart::onWidget3TrainChanged);
    flay->addRow(tr("选择车次"),sels[2]);
    vlay->addLayout(flay);

    table=new QTableView;
    table->setModel(model);
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setSelectionBehavior(QTableView::SelectRows);
    table->setSelectionMode(QTableView::ContiguousSelection);
    vlay->addWidget(table);
    w->setLayout(vlay);
    stack->addWidget(w);
}

RulerPaintPageStart::PaintMode RulerPaintPageStart::getMode()
{
    for(int i=0;i<4;i++){
        if(gpMode->get(i)->isChecked())
            return static_cast<PaintMode>(i);
    }
    return static_cast<PaintMode>(-1);
}

void RulerPaintPageStart::onModeChanged()
{
    stack->setCurrentIndex(getMode());
}

void RulerPaintPageStart::onWidget3TrainChanged(std::shared_ptr<Train> t)
{
    model->setTrain(t);
    table->resizeColumnsToContents();
}
