#include "railtrackwidget.h"

#include <QFormLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHeaderView>
#include <kernel/trackdiagram.h>
#include "data/rail/railstation.h"
#include "data/diagram/diagram.h"

#include <util/qecontrolledtable.h>

#include <model/general/qemoveablemodel.h>
#include "data/common/qesystem.h"

RailTrackSetupWidget::RailTrackSetupWidget(TrackDiagramData &data, QWidget *parent):
    QWidget(parent), _data(data)
{
    initUI();
    refreshData();
}

void RailTrackSetupWidget::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;
    gpMode=new RadioButtonGroup<3,QVBoxLayout>({"手动铺画","双线铺画","单线铺画"},this);
    gpMode->get(0)->setChecked(true);
    gpMode->connectAllTo(SIGNAL(clicked()),this,SLOT(onModeChanged()));
    flay->addRow(tr("铺画模式"),gpMode);
    gpMainStay=new RadioButtonGroup<2>({"允许","不允许"},this);
    gpMainStay->get(1)->setChecked(true);
    flay->addRow(tr("正线停车"),gpMainStay);

    spSame=new QSpinBox;
    spSame->setRange(0,100000);
    spSame->setSingleStep(30);
    spSame->setSuffix(tr(" 秒 (s)"));
    spSame->setMaximumWidth(200);
    flay->addRow(tr("同向接车间隔"),spSame);

    spOpps=new QSpinBox;
    spOpps->setRange(0,100000);
    spOpps->setSingleStep(30);
    spOpps->setSuffix(tr(" 秒 (s)"));
    spOpps->setMaximumWidth(200);
    flay->addRow(tr("对向接车间隔"),spOpps);
    vlay->addLayout(flay);
    vlay->addWidget(new QLabel(tr("股道次序表")));
    ctable=new QEControlledTable();
    table=ctable->table();
    model=new QEMoveableModel(this);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    model->setColumnCount(1);
    model->setHorizontalHeaderLabels({tr("股道名称")});
    table->setModel(model);
    table->setEditTriggers(QTableView::AllEditTriggers);
    vlay->addWidget(ctable);

    auto* g=new ButtonGroup<3>({"铺画","刷新","保存"});
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actApply()),SLOT(refreshData()),
                  SLOT(actSave())});
    vlay->addLayout(g);
}

void RailTrackSetupWidget::refreshData()
{
    if(_data.manual()){
        gpMode->get(0)->setChecked(true);
    }else if(_data.doubleLine()){
        gpMode->get(1)->setChecked(true);
    }else{
        gpMode->get(2)->setChecked(true);
    }

    if(_data.allowMainStay()){
        gpMainStay->get(0)->setChecked(true);
    }else{
        gpMainStay->get(1)->setChecked(true);
    }

    spSame->setValue(_data.sameSplitSecs());
    spOpps->setValue(_data.oppositeSplitSecs());

    model->setRowCount(_data.trackCount());

    int row=0;
    foreach(const auto& p, _data.getTrackOrder()){
        model->setItem(row++,0,new QStandardItem(p));
    }
}

void RailTrackSetupWidget::actApply()
{
    if (gpMode->get(0)->isChecked()){
        _data.setManual(true);
    }else{
        _data.setManual(false);
        if(gpMode->get(1)->isChecked()){
            _data.setDoubleLine(true);
        }else{
            _data.setDoubleLine(false);
        }
    }

    _data.setAllowMainStay (gpMainStay->get(0)->isChecked());
    _data.setSameSplitSecs(spSame->value());
    _data.setOppositeSplitSecs(spOpps->value());

    auto& order=_data.getTrackOrderRef();
    order.clear();
    for(int i=0;i<model->rowCount();i++){
        order.push_back(model->item(i)->text());
    }
    emit applied();
}

void RailTrackSetupWidget::actSave()
{
    QList<QString> order;
    for(int i=0;i<model->rowCount();i++){
        order.push_back(model->item(i)->text());
    }
    emit actSaveOrder(order);
}

void RailTrackSetupWidget::onModeChanged()
{
    if (gpMode->get(0)->isChecked()){
        // 手动模式：禁用正线停车选项
        gpMainStay->setEnabled(false);
        table->setEnabled(true);
    }else{
        // 自动模式：禁用表格
        gpMainStay->setEnabled(true);
        table->setEnabled(false);
    }
}


RailTrackWidget::RailTrackWidget(Diagram &diagram, std::shared_ptr<Railway> railway,
                 std::shared_ptr<RailStation> station, QWidget *parent):
    QSplitter(parent), diagram(diagram), railway(railway), station(station),
    events(diagram.stationTrainsSettled(railway,station)),
    data(events,station->tracks)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog);
    setWindowTitle(tr("车站股道分析 - %1 @ %2").arg(railway->name(),
                                              station->name.toSingleLiteral()));
    resize(1200,600);
    initUI();
}

void RailTrackWidget::initUI()
{
    setupWidget=new RailTrackSetupWidget(data);
    addWidget(setupWidget);
    connect(setupWidget,&RailTrackSetupWidget::applied,
            this,&RailTrackWidget::refreshData);
    connect(setupWidget,&RailTrackSetupWidget::actSaveOrder,
            this,&RailTrackWidget::onSaveOrder);

    auto* w=new QWidget;
    auto* vlay=new QVBoxLayout(w);

    auto* lab=new QLabel(tr("说明：本功能提供股道占用示意图。如果选择“手动模式”，"
                                 "则依据车次时刻表中设置的“股道”安排；否则系统自动安排。"
                                 "系统自动安排的结果仅是一种可能方案，与真实安排方案无关。"
                                 "如果选择“手动模式”，则自动安排的车次一律按照单线且允许正线停车模式。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    slider=new QSlider(Qt::Horizontal);
    slider->setRange(1,120);
    slider->setMaximumWidth(600);
    auto* hlay=new QHBoxLayout;
    hlay->addWidget(new QLabel(tr("水平缩放")));
    hlay->addStretch(10);
    hlay->addWidget(new QLabel(tr("大")));
    hlay->addStretch(2);
    hlay->addWidget(slider);
    hlay->addStretch(2);
    hlay->addWidget(new QLabel(tr("小")));
    hlay->addStretch(20);
    vlay->addLayout(hlay);

    trackDiagram=new TrackDiagram(data);
    vlay->addWidget(trackDiagram);
    addWidget(w);
    slider->setValue(trackDiagram->xScale());
    connect(slider,&QSlider::valueChanged, trackDiagram,
            &TrackDiagram::setXScale);

}

void RailTrackWidget::refreshData()
{
    trackDiagram->refreshData();
}

void RailTrackWidget::onSaveOrder(const QList<QString> &order)
{
    emit actSaveTrackOrder(railway,station,order);
}


