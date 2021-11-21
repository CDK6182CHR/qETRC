#ifdef QETRC_MOBILE


#include "atrainoptions.h"
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>
#include <util/buttongroup.hpp>
#include <util/selecttraincombo.h>
#include <data/diagram/diagram.h>
#include <viewers/events/traineventdialog.h>
#include <data/train/train.h>


ATrainOptions::ATrainOptions(Diagram &diagram, QWidget *parent):
    QWidget(parent), diagram(diagram)
{
    initUI();
}

void ATrainOptions::initUI()
{
    // 0. 搜索
    // 1. 事件表
    // 2. 运行线一览
    // 3. 标尺对照
    // 4. 时刻诊断

    auto* vlay=new QVBoxLayout(this);
    auto* hlay=new QHBoxLayout;
    edSearch=new QLineEdit;
    hlay->addWidget(edSearch);
    auto* btn=new QPushButton(tr("搜索"));
    connect(btn,&QPushButton::clicked,this,&ATrainOptions::actSearch);
    hlay->addWidget(btn);
    vlay->addLayout(hlay);

    auto* g=new ButtonGroup<4,QVBoxLayout>({"事件表","运行线一览","标尺对照","时刻诊断"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SIGNAL(actTrainEvent()),SIGNAL(actTrainLines()),
                  SIGNAL(actRulerRef()),SIGNAL(actDiagnosis())});

}

void ATrainOptions::actSearch()
{
    const auto& t=edSearch->text();
    auto trains=diagram.trainCollection().multiSearchTrain(t);
    if (trains.empty()){
        QMessageBox::warning(this,tr("错误"),tr("无符合条件车次"));
        return;
    }else if(trains.size()==1){
        emit focusInTrain(trains.at(0));
    }else{
        QStringList lst;
        foreach(auto train,trains){
            lst.append(train->trainName().full());
        }
        bool ok;
        auto s=QInputDialog::getItem(this,tr("搜索车次"),
                                     tr("有多个车次符合条件，请选择一个"),lst,0,false,
                                     &ok);
        if(!ok)return;
        auto train=diagram.trainCollection().findFullName(s);
        if(!train){
            QMessageBox::warning(this,tr("错误"),tr("程序错误：非法车次"));
        }else{
            emit focusInTrain(train);
        }
    }
}



#endif


