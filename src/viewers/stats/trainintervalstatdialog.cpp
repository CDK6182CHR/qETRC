#include "trainintervalstatdialog.h"
#include "data/diagram/diagram.h"
#include "util/selecttraincombo.h"
#include "util/utilfunc.h"
#include "data/train/train.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>


TrainIntervalStatDialog::TrainIntervalStatDialog(Diagram& diagram,
                                                 QWidget *parent):
    QDialog(parent),diagram(diagram),stat(diagram.railCategory())
{
    setWindowTitle(tr("列车区间运行统计"));
    resize(600,800);
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

#define SET_ED(_Var,_Key,_Tooltip) do{\
this->_Var=new QLineEdit;\
this->_Var->setReadOnly(true);\
this->_Var->setToolTip(tr(_Tooltip));\
flay->addRow(tr(_Key),_Var);\
}while(false)

void TrainIntervalStatDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;

    cbTrain=new SelectTrainCombo(diagram.trainCollection());
    flay->addRow(tr("车次"),cbTrain);
    connect(cbTrain,&SelectTrainCombo::currentTrainChanged,
            this,&TrainIntervalStatDialog::setTrain);

    cbStart=new QComboBox;
    flay->addRow(tr("起始站"),cbStart);
    connect(cbStart,qOverload<int>(&QComboBox::currentIndexChanged),
            this,&TrainIntervalStatDialog::onStartIndexChanged);

    cbEnd=new QComboBox;
    flay->addRow(tr("终止站"),cbEnd);
    connect(cbEnd,qOverload<int>(&QComboBox::currentIndexChanged),
            this,&TrainIntervalStatDialog::onEndIndexChanged);

    SET_ED(edStations,"图定站点数","时刻表中图定站点数，包括两端");
    SET_ED(edStops,"图定停点数","时刻表中图定停站次数，不包括两端");
    SET_ED(edTotalTime,"总运行时长","包含运行和停站时长（不含两端）");
    SET_ED(edRunTime,"纯运行时长","不含停站时间的纯运行时长");
    SET_ED(edStopTime,"总停站时长","停站时间总计（不包括两端）");
    SET_ED(edMile,"区间里程","线路区间里程。仅在运行图具有充分信息时才能计算。");
    SET_ED(edTravelSpeed,"旅行速度","区间旅行速度，包含运行时长和停站时长（停站不包含两端站）");
    SET_ED(edTechSpeed,"技术速度","区间技术速度，仅包含运行时长");

    vlay->addLayout(flay);

    auto* lab=new QLabel(tr("经由/说明"));
    vlay->addWidget(lab);
    edPath=new QTextBrowser;
    vlay->addWidget(edPath);

    auto* btn=new QPushButton(tr("关闭"));
    vlay->addWidget(btn);
    connect(btn,&QPushButton::clicked,this,&QDialog::close);
}

void TrainIntervalStatDialog::refreshForTrain()
{
    setupStartCombo();
    setupEndCombo(0);
    refreshData();
}

void TrainIntervalStatDialog::setupStartCombo()
{
    if (train->empty()) return;
    cbStart->blockSignals(true);
    cbStart->clear();
    for(const auto& t:train->timetable()){
        cbStart->addItem(t.name.toSingleLiteral());
    }
    cbStart->blockSignals(false);
}

void TrainIntervalStatDialog::setupEndCombo(int startIdx)
{
    if (train->empty()) return;
    cbEnd->blockSignals(true);
    cbEnd->clear();
    endComboStartIndex=startIdx+1;
    auto itr=std::next(train->timetable().begin());
    std::advance(itr,startIdx);
    for(;itr!=train->timetable().end();++itr){
        cbEnd->addItem(itr->name.toSingleLiteral());
    }
    cbEnd->blockSignals(false);
}

int TrainIntervalStatDialog::actualEndIndex()const
{
    return endComboStartIndex + cbEnd->currentIndex();
}

void TrainIntervalStatDialog::setTrain(std::shared_ptr<Train> train)
{
    this->train=train;
    refreshForTrain();
}

void TrainIntervalStatDialog::onStartIndexChanged(int i)
{
    int endidx=actualEndIndex();
    setupEndCombo(i);
    if (endidx > endComboStartIndex){
        // the new index is still valid, set to the same row
        cbEnd->setCurrentIndex(endidx-endComboStartIndex);
        // the data will be freshed by changing endCombo
    }else{
        // the new index is invalid, or the new index is 0
        // the cbEnd will now change, then update manually
        refreshData();
    }
}

void TrainIntervalStatDialog::onEndIndexChanged([[maybe_unused]]int i)
{
    refreshData();
}

void TrainIntervalStatDialog::refreshData()
{
    //guards
    if (!train || train->empty()) return;
    if (cbStart->currentIndex() < 0 || cbEnd->currentIndex() < 0) return;

    int startIdx=cbStart->currentIndex();
    int endIdx=actualEndIndex();

    stat.setTrain(train);
    stat.setRange(startIdx,endIdx);
    auto res=stat.compute();

    edStations->setText(QString::number(res.settledStationsCount));
    edStops->setText(QString::number(res.settledStopCount));
    edTotalTime->setText(qeutil::secsToStringHour(res.totalSecs));
    edRunTime->setText(qeutil::secsToStringHour(res.runSecs));
    edStopTime->setText(qeutil::secsToStringHour(res.stopSecs));

    if (res.railResults.isValid){
        edMile->setText(tr("%1 km").arg(res.railResults.totalMiles,3));
        edTravelSpeed->setText(tr("%1 km/h").arg(res.railResults.travelSpeed,4));
        edTechSpeed->setText(tr("%1 km/h").arg(res.railResults.techSpeed,4));
        edPath->setText(res.railResults.path_s);
    }else{
        edMile->clear();
        edTravelSpeed->clear();
        edTechSpeed->clear();
        edPath->setText(tr("没有完整的线路信息，无法计算里程和速度：\n%1").arg(res.railResults.path_s));
    }
}
