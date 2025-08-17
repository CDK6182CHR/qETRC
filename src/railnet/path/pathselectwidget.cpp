#include "graphpathmodel.h"
#include "pathoperationmodel.h"
#include "pathselectwidget.h"
#include "data/common/qesystem.h"

#include <railnet/graph/adjacentlistmodel.h>

#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QFormLayout>
#include <QMessageBox>

PathSelectWidget::PathSelectWidget(const RailNet &net, QWidget *parent):
    QSplitter(parent),
    net(net),
    seqModel(new PathOperationModel(net,this)),
    adjModel(new AdjacentListModel(net,this)),
    pathModel(new GraphPathModel(this))
{
    initUI();
    setWindowTitle(tr("高级经由选择器"));
}

void PathSelectWidget::initUI()
{
    // 左侧，是一个套娃的splitter
    auto* sp=new QSplitter(Qt::Vertical);
    auto* w=new QWidget;
    auto* vlay=new QVBoxLayout(w);
    auto* hlay=new QHBoxLayout;
    edStation=new QLineEdit;
    hlay->addWidget(edStation);
    auto* btn=new QPushButton(tr("添加关键点"));
    connect(btn,&QPushButton::clicked,this,&PathSelectWidget::addByShortestPath);
    hlay->addWidget(btn);
    vlay->addLayout(hlay);

    // 已选关键点表

    hlay=new QHBoxLayout;
    btn=new QPushButton(tr("删除关键点"));
    connect(btn,&QPushButton::clicked,this,&PathSelectWidget::popSelect);
    hlay->addWidget(btn);
    btn=new QPushButton(tr("清空径路"));
    connect(btn,&QPushButton::clicked,this,&PathSelectWidget::clearSelect);
    hlay->addWidget(btn);
    vlay->addLayout(hlay);

    vlay->addWidget(new QLabel(tr("已选径路关键点表：")));
    seqTable=setupTable();
    seqTable->setModel(seqModel);
    vlay->addWidget(seqTable);
    connect(seqModel,&PathOperationModel::lastVertexChanged,
            this,&PathSelectWidget::onSelectSeqChanged);

    int c=0;
    for(int wd:{100,80,80,200}){
        seqTable->setColumnWidth(c++,wd);
    }

    sp->addWidget(w);

    // 邻接站表

    w=new QWidget;
    vlay=new QVBoxLayout(w);
    vlay->addWidget(new QLabel(tr("当前站邻接站表：")));
    adjTable=setupTable();
    adjTable->setModel(adjModel);
    connect(adjTable->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,&PathSelectWidget::onCurrentAdjStationChanged);
    connect(adjTable,&QTableView::doubleClicked,
            this,&PathSelectWidget::addByAdjStation);
    vlay->addWidget(adjTable);

    c=0;
    for(int wd:{100,80,80,60,60,60})
        adjTable->setColumnWidth(c++,wd);

    btn=new QPushButton(tr("添加至径路关键点表"));
    connect(btn,&QPushButton::clicked,this,&PathSelectWidget::addByAdjStationBtn);
    vlay->addWidget(btn);
    sp->addWidget(w);

    addWidget(sp);

    // 右边：邻接线表
    w=new QWidget;
    vlay=new QVBoxLayout(w);
    auto* flay=new QFormLayout;
    edRailName=new QLineEdit;
    edRailName->setFocusPolicy(Qt::NoFocus);
    flay->addRow(tr("当前线路"),edRailName);
    vlay->addLayout(flay);
    pathTable=setupTable();
    pathTable->setModel(pathModel);
    vlay->addWidget(pathTable);

    c=0;
    for(int wd:{120,80,80,80}){
        pathTable->setColumnWidth(c++,wd);
    }

    connect(pathTable,&QTableView::doubleClicked,
            this,&PathSelectWidget::addByAdjRailway);
    btn=new QPushButton(tr("添加至径路关键点表"));
    connect(btn,&QPushButton::clicked,
            this,&PathSelectWidget::addByAdjRailwayBtn);
    vlay->addWidget(btn);
    addWidget(w);
}

void PathSelectWidget::onSelectSeqChanged(std::shared_ptr<const RailNet::vertex> v)
{
    adjModel->setupForOutAdj(v);
    adjTable->resizeColumnsToContents();

    // 设置邻接表第一行为选中
    if (adjModel->rowCount()>0){
        adjTable->setCurrentIndex(adjModel->index(0,0));
        setAdjStationRow(0);
    }
}

void PathSelectWidget::onCurrentAdjStationChanged(const QModelIndex &idx)
{
    if (!idx.isValid()){
        pathModel->setPath({});
    }
    setAdjStationRow(idx.row());
}

void PathSelectWidget::setAdjStationRow(int row)
{
    auto e=adjModel->edgeFromRow(row);
    auto path=net.railPathFrom(e);
    pathModel->setPath(std::move(path));
    if(e){
        edRailName->setText(tr("%1  (%2)").arg(e->data.railName,
                                               DirFunc::dirToString(e->data.dir)));
        pathTable->resizeColumnsToContents();
    }
}

void PathSelectWidget::addByShortestPath()
{
    const QString& text=edStation->text();
    if(text.isEmpty()){
        QMessageBox::warning(this,tr("错误"),tr("站名不能为空"));
        return;
    }
    QString rep;
    bool flag=seqModel->addByShortestPath(text,&rep);
    if(!flag){
        QMessageBox::warning(this,tr("错误"),tr("关键点添加失败，原因如下:\n %1")
                             .arg(rep));
    }
}

void PathSelectWidget::popSelect()
{
    seqModel->popSelect();
}

void PathSelectWidget::clearSelect()
{
    auto flag=QMessageBox::question(this,tr("提示"),tr("是否确认清空所选路径？"
        "此操作不可撤销。"));
    if(flag==QMessageBox::Yes){
        seqModel->clearSelect();
    }
}

void PathSelectWidget::addByAdjStation(const QModelIndex &idx)
{
    if (!idx.isValid()){
        QMessageBox::warning(this,tr("警告"),tr("非法选择"));
        return;
    }
    auto e=adjModel->edgeFromRow(idx.row());
    if(!e){
        QMessageBox::warning(this,tr("错误"),tr("意外的空邻接表结点"));
        return;
    }
    QString report;
    bool flag=seqModel->addByAdjStation(e, &report);
    if (!flag){
        QMessageBox::warning(this,tr("错误"),tr("添加失败，原因如下：\n%1").arg(report));
    }
}

void PathSelectWidget::addByAdjStationBtn()
{
    addByAdjStation(adjTable->currentIndex());
}

void PathSelectWidget::addByAdjRailway(const QModelIndex &idx)
{
    if (!idx.isValid()){
        QMessageBox::warning(this,tr("警告"),tr("非法选择。"));
        return;
    }
    auto path=pathModel->subPathTo(idx.row());
    if(path.empty()){
        QMessageBox::warning(this,tr("错误"),tr("意外的空径路。\n"
        "注意第1行为当前结点站，不可选择。"));
        return;
    }
    QString report;
    bool flag=seqModel->addByAdjPath(std::move(path),&report);
    if(!flag){
        QMessageBox::warning(this,tr("错误"),tr("按邻线添加失败，原因如下：\n").arg(report));
    }
}

void PathSelectWidget::addByAdjRailwayBtn()
{
    addByAdjRailway(pathTable->currentIndex());
}

QTableView *PathSelectWidget::setupTable()
{
    auto* tab=new QTableView;
    tab->setEditTriggers(QTableView::NoEditTriggers);
    tab->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    tab->setSelectionBehavior(QTableView::SelectRows);
    return tab;
}
