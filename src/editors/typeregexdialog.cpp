#include "typeregexdialog.h"
#include "model/train/typemodel.h"
#include "data/train/traintype.h"
#include "util/qecontrolledtable.h"
#include "data/common/qesystem.h"
#include "util/buttongroup.hpp"
#include <QtWidgets>


TypeRegexDialog::TypeRegexDialog(TypeManager &manager_,bool forDefault_, QWidget *parent):
    QDialog(parent),manager(manager_),forDefault(forDefault_),
    model(new TypeRegexModel(manager_,this))
{
    setWindowTitle(tr("类型规则"));
    resize(600,600);
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void TypeRegexDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel;
    if (forDefault) {
        lab->setText(tr("【注意】当前编辑的是系统默认设置，编辑结果不会直接应用于当前运行图，"
            "而是作为以后新建运行图的缺省配置。数据保存在config.json文件中。"));
    }
    else {
        lab->setText(tr("【注意】当前编辑的运行图类型管理器，编辑结果将直接应用到当前运行图。"));
    }
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    lab=new QLabel(tr("请在下表中编辑车次正则表达式对应的类型，每行判定一个类型。表格有顺序，"
    "每次判定车次时，从上到下，判定到第一个匹配的，即截止。类型名称可以重复；如果类型名称"
    "不存在，则自动创建相应的类型。请至[类型管理]中编辑类型对应的颜色、线型等数据。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    ctab=new QEControlledTable;
    table=ctab->table();
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->setEditTriggers(QTableView::AllEditTriggers);
    vlay->addWidget(ctab);

    auto* g=new ButtonGroup<3>({"确定","还原","取消"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actApply()),SLOT(refreshData()),SLOT(close())});
}

void TypeRegexDialog::actApply()
{
    auto data=std::make_shared<TypeManager>();
    bool flag=model->appliedData(*data);
    if(flag){
        if (data->regex() != manager.regex()){
            emit typeRegexApplied(manager,std::move(data));
        }
    }
}

void TypeRegexDialog::refreshData()
{
    model->refreshData();
}

