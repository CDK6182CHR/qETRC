#include "atrainoptions.h"
#include "atrainpage.h"

#include <QHBoxLayout>
#include <QListView>
#include <QTabWidget>
#include <QScroller>
#include <QMessageBox>

#include <model/train/trainlistmodel.h>
#include <viewers/diagnosisdialog.h>
#include <viewers/rulerrefdialog.h>
#include <viewers/timetablequickwidget.h>
#include <viewers/traininfowidget.h>
#include <viewers/trainlinedialog.h>
#include <data/train/traincollection.h>
#include <data/diagram/diagram.h>
#include <viewers/events/traineventdialog.h>

#ifdef QETRC_MOBILE


ATrainPage::ATrainPage(Diagram& diagram_, QWidget *parent):
    QWidget(parent),diagram(diagram_), coll(diagram_.trainCollection())
{
    initUI();
}

void ATrainPage::initUI()
{
    auto* hlay=new QHBoxLayout(this);

    lsTrains=new QListView;
    QScroller::grabGesture(lsTrains,QScroller::TouchGesture);
    model=new TrainListModel(coll,nullptr,this);
    lsTrains->setModel(model);
    hlay->addWidget(lsTrains);

    tab=new QTabWidget();

    timetable=new TimetableQuickWidget(nullptr);
    timetable->setReadonly();
    tab->addTab(timetable,tr("时刻表"));

    info=new TrainInfoWidget();
    tab->addTab(info,tr("信息"));
    hlay->addWidget(tab);
    tab->setTabPosition(QTabWidget::East);

    connect(lsTrains->selectionModel(),&QItemSelectionModel::currentRowChanged,
            this,&ATrainPage::onCurrentRowChanged);

    opt=new ATrainOptions(diagram);
    tab->addTab(opt,tr("操作"));
    connect(opt,&ATrainOptions::actTrainEvent,
            this,&ATrainPage::actTrainEvent);
    connect(opt,&ATrainOptions::actTrainLines,
            this,&ATrainPage::actTrainLines);
    connect(opt,&ATrainOptions::actRulerRef,
            this,&ATrainPage::actRulerRef);
    connect(opt,&ATrainOptions::actDiagnosis,
            this,&ATrainPage::actDiagnosis);
    connect(opt,&ATrainOptions::focusInTrain,
            this,&ATrainPage::setTrain);
}

void ATrainPage::setTrain(std::shared_ptr<Train> train)
{
    this->train=train;
    timetable->setTrain(train);
    info->setTrain(train);
}

void ATrainPage::refreshData()
{
    setTrain(nullptr);
    model->refreshData();
}

void ATrainPage::onCurrentRowChanged(const QModelIndex &idx)
{
    if (!idx.isValid()) return;
    auto t=coll.trainAt(idx.row());
    setTrain(t);
}

void ATrainPage::actTrainEvent()
{
    if(!train){
        QMessageBox::warning(this,tr("错误"),"当前没有选中车次!");
        return;
    }
    auto* w=new TrainEventDialog(diagram,train,this);
    w->showMaximized();
}

void ATrainPage::actTrainLines()
{
    if(!train){
        QMessageBox::warning(this,tr("错误"),"当前没有选中车次!");
        return;
    }
    auto* w=new TrainLineDialog(train,this);
    w->showMaximized();
}

void ATrainPage::actRulerRef()
{
    if(!train){
        QMessageBox::warning(this,tr("错误"),"当前没有选中车次!");
        return;
    }
    auto* w=new RulerRefDialog(train,this);
    w->showMaximized();
}

void ATrainPage::actDiagnosis()
{
    if(!train){
        QMessageBox::warning(this,tr("错误"),"当前没有选中车次!");
        return;
    }
    auto* w=new DiagnosisDialog(diagram,train,this);
    w->showMaximized();
}


#endif
