#include "railtrackwidget.h"

#include <QFormLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <kernel/trackdiagram.h>
#include "data/rail/railstation.h"
#include "data/diagram/diagram.h"
#include "data/train/trainstation.h"
#include <util/qecontrolledtable.h>

#include <model/general/qemoveablemodel.h>
#include "data/common/qesystem.h"

#include <model/rail/railtrackadjustmodel.h>

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
    auto* btn=new QPushButton(tr("调整"));
    connect(btn,&QPushButton::clicked,this,&RailTrackSetupWidget::actAdjust);
    flay->addRow(tr("股道次序表"),btn);
    vlay->addLayout(flay);
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
        model->setItem(row++,0,new QStandardItem(p->name()));
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

    QStringList order;
    for(int i=0;i<model->rowCount();i++){
        order.push_back(model->item(i)->text());
    }
    _data.setInitOrder(order);
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

void RailTrackSetupWidget::actAdjust()
{
    if(informOnAdjust){
        QMessageBox::information(this,tr("提示"),tr("此功能提供股道名称和次序的调整，"
        "调整过程中股道中的车次序列保持不变。自动、手动铺画模式下，皆可使用。\n"
        "请注意所有调整立即生效且不能撤销，无论是否重新进行铺画。"
        "请注意数据有效性。设置空白或冲突的股道名称可能导致未定义行为。"));
        informOnAdjust=false;
    }
    auto* dlg=new TrackAdjustDialog(_data.getTrackOrderRef(),this);
    connect(dlg,&TrackAdjustDialog::repaintDiagram,
            this,&RailTrackSetupWidget::repaintDiagram);
    dlg->open();   // modal ..
}


RailTrackWidget::RailTrackWidget(Diagram &diagram, std::shared_ptr<Railway> railway,
                 std::shared_ptr<RailStation> station, QWidget *parent):
    QSplitter(parent), diagram(diagram), railway(railway), station(station),
    events(diagram.stationTrainsSettled(railway,station)),
    data(events,station->tracks)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog);
    setWindowTitle(tr("车站股道分析 - %2 @ %1").arg(railway->name(),
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

    hlay=new QHBoxLayout;
    auto* btn=new QPushButton(tr("保存股道分析结果至时刻表"));
    btn->setMaximumWidth(200);
    connect(btn,&QPushButton::clicked,this,
            &RailTrackWidget::actSaveTracks);
    hlay->addWidget(btn);
    vlay->addLayout(hlay);

    trackDiagram=new TrackDiagram(data);
    vlay->addWidget(trackDiagram);
    addWidget(w);
    slider->setValue(trackDiagram->xScale());
    connect(slider,&QSlider::valueChanged, trackDiagram,
            &TrackDiagram::setXScale);

    connect(setupWidget,&RailTrackSetupWidget::repaintDiagram,
            trackDiagram,&TrackDiagram::repaintDiagram);

}

void RailTrackWidget::refreshData()
{
    trackDiagram->refreshData();
    setupWidget->refreshData();
}

void RailTrackWidget::onSaveOrder(const QList<QString> &order)
{
    emit actSaveTrackOrder(railway,station,order);
}

void RailTrackWidget::actSaveTracks()
{
    auto flag=QMessageBox::question(this,tr("提醒"),tr("此操作将把当前车站的"
    "股道分析结果保存到列车时刻表，并覆盖时刻表中原有股道信息。\n"
    "重要：请保证从进行本次股道分析起至现在，没有进行过影响本站车次时刻表的操作（例如，"
    "修改通过本站列车的时刻表；删除通过本站的列车等），否则可能导致非预期的结果，"
    "甚至程序崩溃！！\n"
    "是否确认？"));
    if(flag!=QMessageBox::Yes)
        return;
    QVector<TrainStation*> stations;
    QVector<QString> trackNames;
    foreach(const auto& track, data.getTrackOrder()){
        for(auto p=track->begin();p!=track->end();++p){
            if (! p->fromLeft){  // 此条件用来保证不重复
                // 第一个应该总是有值
                if (p->item->trainStation1.value()->track != track->name()){
                    stations.push_back(p->item->trainStation1.value());
                    trackNames.push_back(track->name());
                }
                if(p->item->trainStation2.has_value() &&
                        p->item->trainStation2.value()->track != track->name()){
                    stations.push_back(p->item->trainStation2.value());
                    trackNames.push_back(track->name());
                }
            }
        }
    }
    if(!stations.empty()){
        emit saveTrackToTimetable(stations, trackNames);
        QMessageBox::information(this,tr("提示"),tr("成功保存%1条数据至列车时刻表。"
            "如有问题，此操作可以撤销。").arg(stations.size()));
    }else{
        QMessageBox::information(this,tr("提示"),tr("未做任何更改。"));
    }
}



TrackAdjustDialog::TrackAdjustDialog(QVector<std::shared_ptr<Track> > &order,
                                     QWidget *parent):
    QDialog(parent), order(order), model(new RailTrackAdjustModel(order,this))
{
    setWindowTitle(tr("股道名称调整"));
    resize(400,500);
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void TrackAdjustDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("本功能支持调整股道名称和次序；股道内的车次铺排不变。"
        "请在下表中进行上移、下移或重命名（双击）操作。\n"
        "请注意操作立即生效且不可撤销（即使没有重新铺画）；请勿将股道重命名为空白或冲突名称，"
        "否则将引发未定义行为。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->setEditTriggers(QTableView::DoubleClicked);
    vlay->addWidget(table);

    auto* g=new ButtonGroup<2>({"上移","下移"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(moveUp()),SLOT(moveDown())});

    g=new ButtonGroup<2>({"铺画","关闭"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SIGNAL(repaintDiagram()),SLOT(close())});
}

void TrackAdjustDialog::moveUp()
{
    const auto& idx=table->currentIndex();
    if(idx.isValid())
        model->moveUp(idx.row());
}

void TrackAdjustDialog::moveDown()
{
    const auto& idx=table->currentIndex();
    if(idx.isValid())
        model->moveDown(idx.row());
}
