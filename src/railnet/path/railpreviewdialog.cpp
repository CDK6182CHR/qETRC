#include "railpreviewdialog.h"
#include "model/rail/railstationmodel.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTextBrowser>

#include <util/qecontrolledtable.h>
#include "data/common/qesystem.h"
#include <util/buttongroup.hpp>
#include <data/rail/railway.h>
#include <model/delegate/generaldoublespindelegate.h>
#include <model/delegate/combodelegate.h>
#include <util/dialogadapter.h>

RailPreviewDialog::RailPreviewDialog(QWidget *parent):
    QDialog(parent), model(new RailStationModel(true,this))
{
    resize(600,700);
    setWindowTitle(tr("径路预览"));
    initUI();
}

void RailPreviewDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("以下是生成的线路信息，可进行修改。修改直接生效，且不可撤销。"
        "点击[确定]将生成的线路"
        "添加到当前运行图中，[取消]以终止本次会话。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* flay=new QFormLayout;
    edName=new QLineEdit;
    auto* hlay = new QHBoxLayout;
    hlay->addWidget(edName);
    auto* btn = new QPushButton(tr("查看经由"));
    hlay->addWidget(btn);
    flay->addRow(tr("线路名称"), hlay);
    connect(btn, &QPushButton::clicked, this, &RailPreviewDialog::actShowPath);
    vlay->addLayout(flay);

    ctable=new QEControlledTable;
    table=ctable->table();
    vlay->addWidget(ctable);

    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setEditTriggers(QTableView::AllEditTriggers);
    table->setModel(model);
    table->setItemDelegateForColumn(RailStationModel::ColMile,
                                    new GeneralDoubleSpinDelegate(this));
    table->setItemDelegateForColumn(RailStationModel::ColDir,
                                    new ComboDelegate({tr("不通过"),tr("下行"),tr("上行"),
                                                      tr("上下行")},this));

    int c=0;
    for(int w:{120,80,80,40,40,70,40,40}){
        table->setColumnWidth(c++,w);
    }

    auto* g=new ButtonGroup<2>({"确定","取消"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actConfirm()),SLOT(close())});
}

void RailPreviewDialog::actConfirm()
{
    if(!railway)
        return;
    railway->setName(edName->text());

    bool flag=model->applyChange();
    if(flag){
        emit railConfirmed(railway);
        railway.reset();   // 放弃所有权，使得当前对象被删除
        done(Accepted);
    }else{
        // 已经警告过了
    }
}

void RailPreviewDialog::setRailway(std::shared_ptr<Railway> railway, const QString& pathString)
{
    this->railway=railway;
    this->pathString = pathString;
    model->setRailway(railway);
    refreshBasicData();
}

void RailPreviewDialog::actShowPath()
{
    if (!pathBrowser) {
        pathBrowser = new QTextBrowser();
        pathBrowser->setWindowFlags(Qt::Dialog);
        pathBrowser->setWindowTitle(tr("经由查看"));
        pathDlg = new DialogAdapter(pathBrowser, this);
        pathDlg->setAttribute(Qt::WA_DeleteOnClose, false);
    }
    pathBrowser->setText(pathString);
    pathDlg->show();
}

void RailPreviewDialog::refreshData()
{
    refreshBasicData();
    model->refreshData();
}

void RailPreviewDialog::refreshBasicData()
{
    if(railway){
        edName->setText(railway->name());
    }   else{
        edName->clear();
    }
}
