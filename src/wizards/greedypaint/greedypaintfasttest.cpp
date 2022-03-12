#include "greedypaintfasttest.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QTextBrowser>
#include <QTimeEdit>
#include <QVBoxLayout>

#include <util/buttongroup.hpp>
#include <util/railrulercombo.h>
#include <util/selectrailwaycombo.h>
#include <data/diagram/diagram.h>
#include <data/train/trainname.h>
#include <data/train/train.h>

#include <chrono>

GreedyPaintFastTest::GreedyPaintFastTest(Diagram& diagram_, QWidget *parent):
    QDialog(parent),diagram(diagram_), painter(diagram_)
{
    setWindowTitle(tr("贪心推线 - 快速测试"));
    resize(800, 800);
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void GreedyPaintFastTest::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;

    cbRuler=new RailRulerCombo(diagram.railCategory());
    flay->addRow(tr("排图线路"), cbRuler);
    vlay->addLayout(flay);

    edTrainName=new QLineEdit;
    flay->addRow(tr("车次"), edTrainName);

    edTime=new QTimeEdit;
    edTime->setDisplayFormat("hh:mm:ss");
    flay->addRow(tr("锚点时刻"),edTime);

    spInt=new QSpinBox;
    spInt->setSuffix(tr(" 秒 (s)"));
    spInt->setSingleStep(30);
    spInt->setRange(0,10*3600-1);
    spInt->setValue(6*60);
    flay->addRow(tr("列车间隔"),spInt);

    spBack=new QSpinBox;
    spBack->setRange(0,10000000);
    spBack->setValue(3);
    flay->addRow(tr("最大尝试回溯次数"),spBack);

    auto* hlay=new QHBoxLayout;
    ckDown=new QCheckBox(tr("下行"));
    ckDown->setChecked(true);
    hlay->addWidget(ckDown);

    ckSingle=new QCheckBox(tr("单线"));
    hlay->addWidget(ckSingle);

    ckForbid=new QCheckBox(tr("启用天窗"));
    hlay->addWidget(ckForbid);

    ckStarting=new QCheckBox(tr("在本线始发"));
    hlay->addWidget(ckStarting);
    ckTerminal=new QCheckBox(tr("在本线终到"));
    hlay->addWidget(ckTerminal);
    vlay->addLayout(hlay);

    txtOut=new QTextBrowser;
    vlay->addWidget(txtOut);

    auto* g=new ButtonGroup<2>({"排图","关闭"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(onApply()),SLOT(close())});
}

void GreedyPaintFastTest::onApply()
{
    auto rail=cbRuler->railway();
    auto ruler=cbRuler->ruler();
    if(!rail || rail->empty() || ! ruler){
        QMessageBox::warning(this,tr("错误"),tr("未选中线路或标尺，或空线路！"));
        return;
    }
    TrainName tn(edTrainName->text());
    if(!diagram.trainCollection().trainNameIsValid(tn,nullptr)){
        QMessageBox::warning(this,tr("错误"),tr("非法车次"));
        return;
    }

    auto & cns=painter.constraints();
    painter.setDir(DirFunc::fromIsDown(ckDown->isChecked()));
    cns.setSingleLine(ckSingle->isChecked());
    painter.setLocalStarting(ckStarting->isChecked());
    painter.setLocalTerminal(ckTerminal->isChecked());
    painter.setAnchorTime(edTime->time());
    painter.setRailway(rail);
    painter.setRuler(ruler);
    painter.setMaxBackoffTimes(spBack->value());

    painter.usedForbids().clear();
    if(ckForbid->isChecked()){
        foreach(auto forbid, rail->forbids()){
            painter.usedForbids().push_back(forbid);
        }
    }

    auto allGaps=TrainGap::allPossibleGaps(cns.isSingleLine());
    for(auto gap:allGaps){
        cns[gap]=spInt->value();
    }

    if(ckDown->isChecked()){
        painter.setAnchor(rail->stations().front());
        painter.setEnd(rail->stations().back());
        painter.setStart(rail->stations().front());
    }else{
        painter.setAnchor(rail->stations().back());
        painter.setEnd(rail->stations().front());
        painter.setStart(rail->stations().back());
    }


    using namespace std::chrono_literals;
    auto tm_start=std::chrono::system_clock::now();
    bool res=painter.paint(tn);
    auto tm_end=std::chrono::system_clock::now();
    emit showStatus(tr("自动推线 用时 %1 毫秒").arg((tm_end-tm_start)/1ms));

    // 整理报告
    QString report;

    int i=0;
    for(const auto& t:painter.logs()){
        report.append(tr("%1. %2\n").arg(++i).arg(t->toString()));
    }

    txtOut->setText(report);

    if(auto train=painter.train();!train->empty()){
        diagram.updateTrain(train);
        emit trainAdded(train);
    }


    if (res){
        QMessageBox::information(this,tr("提示"),tr("排图成功"));
    }else{
        QMessageBox::warning(this,tr("提示"),tr("排图失败，可能因为没有满足约束条件的线位。"
        "已保留最后尝试状态。"));
    }

}
