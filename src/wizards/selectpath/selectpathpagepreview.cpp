#include "selectpathpagepreview.h"

#include <model/rail/railstationmodel.h>
#include <data/common/qesystem.h>
#include <QFormLayout>
#include <QLineEdit>
#include <QLineEdit>
#include <QSplitter>
#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextBrowser>
#include <QMessageBox>
#include <util/qecontrolledtable.h>
#include <data/rail/railway.h>
#include <railnet/graph/railnet.h>
#include <railnet/path/pathoperation.h>

SelectPathPagePreview::SelectPathPagePreview(const RailNet &net,
                                             const PathOperationSeq &seqDown,
                                             const PathOperationSeq &seqUp,
                                             QWidget *parent):
    QWizardPage(parent), net(net), seqDown(seqDown), seqUp(seqUp),
    model(new RailStationModel(true, this))
{
    initUI();
}

void SelectPathPagePreview::setupData(bool withRuler, int rulerCount)
{
    auto rdown=net.singleRailFromPathOperations(seqDown,withRuler);
    if (!rdown){
        QMessageBox::warning(this,tr("错误"),tr("意外的空下行线数据"));
        return;
    }
    auto rup=net.singleRailFromPathOperations(seqUp,withRuler);
    if (rup){
        rdown->mergeCounter(*rup);
    }
    rdown->filtRulerByCount(rulerCount);
    this->railway=std::move(rdown);
    refreshData();

    QString text=tr("下行经由：\n%1\n\n上行经由：\n%2\n")
            .arg(net.pathToString(seqDown.fullPath()),
                 net.pathToString(seqUp.fullPath()));
    edPath->setText(text);
}

void SelectPathPagePreview::refreshData()
{
    model->setRailway(railway);
    if(railway){
        edName->setText(railway->name());
        table->resizeColumnsToContents();
    }
}

void SelectPathPagePreview::initUI()
{
    setTitle(tr("预览"));
    setSubTitle(tr("以下是根据所选正向和反向径路生成的线路数据。"
        "生成的数据不一定符合要求，可在下面修改线路以及线名，注意修改不可撤销。"
        "点击[保存]确认修改情况，[完成]结束向导，并将新线路添加到运行图。"));

    auto* sp=new QSplitter;
    sp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* w=new QWidget;
    auto* vlay=new QVBoxLayout(w);
    vlay->setContentsMargins(0,0,0,0);
    auto* flay=new QFormLayout;

    edName=new QLineEdit;
    flay->addRow(tr("线名"),edName);
    vlay->addLayout(flay);

    ctable=new QEControlledTable;
    table=ctable->table();
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setEditTriggers(QTableView::AllEditTriggers);
    table->setModel(model);
    vlay->addWidget(ctable);
    sp->addWidget(w);

    auto* hlay=new QHBoxLayout;
    auto* btn=new QPushButton(tr("保存"));
    connect(btn,&QPushButton::clicked,this,&SelectPathPagePreview::actSave);
    hlay->addWidget(btn);
    btn=new QPushButton(tr("还原"));
    connect(btn,&QPushButton::clicked,model,&RailStationModel::refreshData);
    hlay->addWidget(btn);
    vlay->addLayout(hlay);
    sp->addWidget(w);

    edPath=new QTextBrowser;
    sp->addWidget(edPath);

    auto* topLay=new QVBoxLayout(this);
    topLay->addWidget(sp);
}

bool SelectPathPagePreview::applyChange()
{
    if(!railway) return false;
    railway->setName(edName->text());
    return model->applyChangeInplace();
}

void SelectPathPagePreview::actSave()
{
    applyChange();
}
