﻿#include "typeconfigdialog.h"

#include "model/train/typemodel.h"
#include "data/common/qesystem.h"
#include "util/buttongroup.hpp"
#include "model/delegate/generaldoublespindelegate.h"
#include "data/train/traintype.h"
#include "data/train/typemanager.h"
#include "mainwindow/version.h"
#include "defines/icon_specs.h"
#include <model/delegate/linestyledelegate.h>

#include <util/qecontrolledtable.h>

#include <QLabel>
#include <QHeaderView>
#include <QTableView>
#include <QMessageBox>
#include <QColorDialog>
#include <QCheckBox>
#include <QToolButton>
#include <QApplication>
#include <QStyle>
#include <QDesktopServices>

TypeConfigDialog::TypeConfigDialog(TypeManager &manager_, bool forDefault_, QWidget *parent):
    QDialog(parent), manager(manager_),
    model(new TypeConfigModel(manager_,this)),forDefault(forDefault_)
{
    resize(600,600);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("类型管理"));
    initUI();
}


void TypeConfigDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel;
    if (forDefault) {
        lab->setText(tr("【注意】当前编辑的是系统默认类型管理器，编辑结果不会直接应用到"
            "当前运行图中，而是作为以后新建运行图的缺省配置。"));
    }
    else {
        lab->setText(tr("【注意】当前编辑的是运行图类型管理器，编辑结果将直接应用到"
            "当前的运行图中。"));
        auto* hlay = new QHBoxLayout;
        ckTransparent = new QCheckBox(tr("当前类型管理器为透明状态"));
        ckTransparent->setEnabled(false);
        ckTransparent->setChecked(manager.isTransparent());
        hlay->addWidget(ckTransparent);
        auto* tb = new QToolButton;
        tb->setIcon(QEICN_type_config_transparent_help);
        hlay->addWidget(tb);
        connect(tb, &QToolButton::clicked, this, &TypeConfigDialog::informTransparent);
        vlay->addLayout(hlay);

    }
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    lab=new QLabel(tr("注意：\n"
    "（1）除第1行保证为默认的类型信息外，其他行的顺序无效，即使使用上移、下移来调整，"
    "这个顺序也不会被实际保存。不建议修改默认类型的名称，更不建议删除（除非你看过源码确认安全），"
    "否则可能会导致未定义行为，后果自负。\n"
    "（2）这里删除的类型并不一定真正被删除。如果当前存在车次的类型为被删除的类型，"
    "或正则查找表中有被删除的类型，它都不会被真正删除。\n"
    "（3）虽然支持，建议慎用增删功能。特别是，如果先删除后增加一个同名的类型，而该类型"
    "存在车次，则新编辑的信息无效。\n"
    "（4）类型名不可以重复。如果重复，当前表中排序靠后的起作用。所有结果以提交之后刷新的页面为准。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    ctab=new QEControlledTable;
    table=ctab->table();
    table->setModel(model);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);

    int c=0;
    for(int t:{120,50,120,80,80}){
        table->setColumnWidth(c++,t);
    }

    table->setEditTriggers(QTableView::AllEditTriggers);
    table->setItemDelegateForColumn(TypeConfigModel::ColWidth,
                                    new GeneralDoubleSpinDelegate(this,2));
    table->setItemDelegateForColumn(TypeConfigModel::ColLineStyle,
                                    new LineStyleDelegate(this));
    connect(table,&QTableView::doubleClicked,this,&TypeConfigDialog::onDoubleClicked);

    vlay->addWidget(ctab);

    auto* g=new ButtonGroup<3>({"确定","还原","取消"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actApply()),SLOT(refreshData()),
                                          SLOT(close())});
}

void TypeConfigDialog::actApply()
{
    auto [data, modified] = model->appliedData();
    if (data != manager.types() || !modified.empty()) {
        emit typeSetApplied(manager, data, modified);
        QMessageBox::information(this, tr("提示"), tr("应用成功"));
    }
    else {
        QMessageBox::information(this, tr("提示"), tr("数据没有改变"));
        refreshData();
    }
}

void TypeConfigDialog::onDoubleClicked(const QModelIndex &idx)
{
    if(idx.column() == TypeConfigModel::ColColor){
        auto* it=model->item(idx.row(),idx.column());
        auto color=QColorDialog::getColor(it->background().color(),this,
            tr("类型颜色 - %1").arg(model->item(idx.row(),TypeConfigModel::ColName)->text()));
        if(color.isValid()){
            it->setText(color.name().toUpper());
            it->setBackground(color);
        }
    }
}

void TypeConfigDialog::informTransparent()
{
    QString doc_url = QString("%1/view/view.html#sec-transparent-config").arg(qespec::DOC_URL_PREFIX.data());
    QDesktopServices::openUrl(doc_url);
    //QMessageBox::information(this, tr("透明模式"),
    //    tr("自V1.4.0起，类型管理器默认采用透明模式。\n在透明模式中，每次读取运行图文件时，"
    //        "自动读取当前的系统默认类型管理器，并随系统默认管理器设置变化而变化。\n"
    //        "若用户手动修改类型管理设置并应用，则透明模式关闭。目前透明模式不支持手动设置。\n"
    //    "详见%1/view/view.html#sec-transparent-config").arg(qespec::DOC_URL_PREFIX.data()));
}

void TypeConfigDialog::refreshData()
{
    model->refreshData();
    if (!forDefault) {
        ckTransparent->setChecked(manager.isTransparent());
    }
}
