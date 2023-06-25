#include "typeregexdialog.h"
#include "model/train/typemodel.h"
#include "data/train/traintype.h"
#include "data/train/typemanager.h"
#include "util/qecontrolledtable.h"
#include "data/common/qesystem.h"
#include "util/buttongroup.hpp"
#include "mainwindow/version.h"

#include <QLabel>
#include <QTableView>
#include <QHeaderView>
#include <QCheckBox>
#include <QToolButton>
#include <QApplication>
#include <QStyle>
#include <QMessageBox>
#include <QDesktopServices>

TypeRegexDialog::TypeRegexDialog(TypeManager &manager_,bool forDefault_, QWidget *parent):
    QDialog(parent),manager(manager_),
    model(new TypeRegexModel(manager_,this)),forDefault(forDefault_)
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
        auto* ckTransparent = new QCheckBox(tr("当前类型管理器为透明状态"));
        ckTransparent->setEnabled(false);
        auto* hlay = new QHBoxLayout;
        hlay->addWidget(ckTransparent);
        auto* tb = new QToolButton;
        tb->setIcon(qApp->style()->standardIcon(QStyle::SP_MessageBoxQuestion));
        hlay->addWidget(tb);
        connect(tb, &QToolButton::clicked, this, &TypeRegexDialog::informTransparent);
        vlay->addLayout(hlay);

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
    if (!forDefault) {
        ckTransparent->setChecked(manager.isTransparent());
    }
}

void TypeRegexDialog::informTransparent()
{
    QString doc_url = QString("%1/view/view.html#sec-transparent-config").arg(qespec::DOC_URL_PREFIX.data());
    QDesktopServices::openUrl(doc_url);
    //QMessageBox::information(this, tr("透明模式"),
    //    tr("自V1.4.0起，类型管理器默认采用透明模式。\n在透明模式中，每次读取运行图文件时，"
    //        "自动读取当前的系统默认类型管理器，并随系统默认管理器设置变化而变化。\n"
    //        "若用户手动修改类型管理设置并应用，则透明模式关闭。目前透明模式不支持手动设置。\n"
    //        "详见 <a href=\"%1\"> Doc </a>").arg(doc_url));
}

