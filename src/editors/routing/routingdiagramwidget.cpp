#include "routingdiagramwidget.h"
#include "kernel/routingdiagram.h"
#include "data/train/routing.h"
#include "util/buttongroup.hpp"

#include <QSplitter>
#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QCheckBox>
#include <algorithm>
#include <QFileDialog>
#include <QMessageBox>

#include <data/common/qesystem.h>

#include <util/qecontrolledtable.h>
#include "model/delegate/postivespindelegate.h"

RoutingStationModel::RoutingStationModel(QObject *parent):
    QEMoveableModel(parent)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({tr("站名"),tr("纵坐标")});
}

void RoutingStationModel::setupNewRow(int row)
{
    setItem(row,ColName,new QStandardItem);
    auto* it=new QStandardItem;
    it->setData(0,Qt::EditRole);
    setItem(row,ColYValue,it);
}

void RoutingStationModel::setupModel(const QMap<StationName, double> &yvalues)
{
    using SI=QStandardItem;
    QVector<QPair<double ,StationName>> yvecs;
    yvecs.reserve(yvalues.size());
    for(auto p=yvalues.begin();p!=yvalues.end();++p){
        yvecs.push_back(qMakePair(p.value(),p.key()));
    }
    std::sort(yvecs.begin(),yvecs.end());
    setRowCount(yvecs.size());
    for(int i=0;i<yvecs.size();i++){
        setItem(i,ColName,new SI(yvecs.at(i).second.toSingleLiteral()));
        auto* it=new SI;
        it->setData(yvecs.at(i).first,Qt::EditRole);
        setItem(i,ColYValue,it);
    }
}

void RoutingStationModel::getData(QMap<StationName, double> &yvalues)
{
    yvalues.clear();
    for(int i=0;i<rowCount();i++){
        yvalues.insert(item(i,ColName)->text(),item(i,ColYValue)->data(Qt::EditRole).toDouble());
    }
}



RoutingDiagramWidget::RoutingDiagramWidget(std::shared_ptr<Routing> routing_, QWidget *parent):
    QSplitter(parent),routing(routing_),routingDiagram(new RoutingDiagram(routing_)),
    model(new RoutingStationModel(this))
{
    connect(routingDiagram,&RoutingDiagram::diagramRepainted,
            this,&RoutingDiagramWidget::_updateTable);
    resize(1300, 870);
    initUI();
    _updateTable();
}

void RoutingDiagramWidget::initUI()
{
    setWindowTitle(tr("交路示意图 - %1").arg(routing->name()));

    auto* w=new QWidget;
    auto* vlay=new QVBoxLayout(w);
    ctab=new QEControlledTable;
    table=ctab->table();
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->setItemDelegateForColumn(RoutingStationModel::ColYValue,
                                    new PostiveSpinDelegate(10,this));
    table->setEditTriggers(QTableView::AllEditTriggers);

    vlay->addWidget(ctab);
    auto* g=new ButtonGroup<2>({"重新铺画","自动铺画"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(_repaint()),
                  SLOT(_autoPaint())});
    addWidget(w);

    w=new QWidget;
    vlay=new QVBoxLayout(w);
    auto* flay=new QFormLayout;
    auto* lab=new QLabel(tr("说明：若勾选“扫描整个时刻表”，则系统遍历时刻表中的每一个站，当下一时刻"
                                 "在前一时刻之前时，认为跨日，此情况下要求整个时刻表不能有错。"
                                 "若不勾选，则仅比较最后时刻和最前时刻，当后者在前者之前时认为跨日。"
                                 "此情况下仅在车次跨最多一日时才能正确处理。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    ckTimetable=new QCheckBox(tr("扫描整个时刻表"));
    connect(ckTimetable,&QCheckBox::toggled,
            routingDiagram,&RoutingDiagram::setMultiDayByTimetable);
    flay->addRow(tr("跨日算法"),ckTimetable);

    auto* slider=new QSlider(Qt::Horizontal);
    slider->setMaximumWidth(400);
    slider->setRange(100,2000);
    flay->addRow(tr("水平缩放"),slider);
    slider->setValue(routingDiagram->getSizes().dayWidth);
    connect(slider,&QSlider::valueChanged,
            routingDiagram,&RoutingDiagram::setDayWidth);

    slider=new QSlider(Qt::Horizontal);
    slider->setMaximumWidth(400);
    slider->setRange(200,800);
    flay->addRow(tr("垂直缩放"),slider);
    slider->setValue(routingDiagram->getSizes().height);
    connect(slider,&QSlider::valueChanged,routingDiagram,
            &RoutingDiagram::setHeight);
    g=new ButtonGroup<2>({"PNG图片","PDF文档"});
    g->setMaximumWidth(180);
    flay->addRow(tr("导出为"),g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(_outPng()),
                  SLOT(_outPdf())});
    vlay->addLayout(flay);
    vlay->addWidget(routingDiagram);
    addWidget(w);
}

void RoutingDiagramWidget::_updateTable()
{
    model->setupModel(routingDiagram->getYValues());
}

void RoutingDiagramWidget::_repaint()
{
    model->getData(routingDiagram->getUserYValues());
    routingDiagram->refreshDiagram();
}

void RoutingDiagramWidget::_autoPaint()
{
    routingDiagram->getUserYValues().clear();
    routingDiagram->refreshDiagram();
}


void RoutingDiagramWidget::_outPdf()
{
    QString f=QFileDialog::getSaveFileName(this,tr("输出PDF"),routing->name(),
                                 tr("PDF文档(*.pdf)"));
    if(f.isEmpty())return;
    bool flag=routingDiagram->outVector(f);
    if(flag){
        QMessageBox::information(this,tr("提示"),tr("导出PDF交路图成功"));
    }else{
        QMessageBox::warning(this,tr("错误"),tr("导出失败，可能因为文件冲突。"));
    }
}

void RoutingDiagramWidget::_outPng()
{
    QString f=QFileDialog::getSaveFileName(this,tr("输出PNG"),routing->name(),
                                 tr("PNG图片(*.png)"));
    if(f.isEmpty())return;
    bool flag=routingDiagram->outPixel(f);
    if(flag){
        QMessageBox::information(this,tr("提示"),tr("导出PNG交路图成功"));
    }else{
        QMessageBox::warning(this,tr("错误"),tr("导出失败，可能因为文件冲突。"));
    }
}

