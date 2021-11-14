#include "astartpage.h"

#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

#include <util/buttongroup.hpp>
#include <mainwindow/version.h>

#ifdef QETRC_MOBILE

AStartPage::AStartPage(QWidget *parent) : QWidget(parent)
{
    initUI();
}

void AStartPage::initUI()
{
    auto* vlay=new QVBoxLayout(this);

    auto* lab=new QLabel(tr("qETRC-Mobile"));
    lab->setAlignment(Qt::AlignCenter);
    vlay->addWidget(lab);

    lab=new QLabel(tr("当前运行图文件"));
    lab->setAlignment(Qt::AlignCenter);
    vlay->addWidget(lab);

    edDiaName=new QLineEdit;
    edDiaName->setFocusPolicy(Qt::NoFocus);
    vlay->addWidget(edDiaName);

    auto* g=new ButtonGroup<2>({"打开","重置"});
    vlay->addLayout(g);

    g->connectAll(SIGNAL(clicked()),this,{SLOT(actOpen()),SLOT(actClear())});

    lab=new QLabel(tr("qETRC_%1_%2").arg(qespec::VERSION.data(),
                   qespec::DATE.data()));
    lab->setAlignment(Qt::AlignCenter);
    vlay->addWidget(lab);

    lab=new QLabel(tr("qETRC移动版目前仅提供部分查看功能，不提供编辑功能。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

}

void AStartPage::actOpen()
{
    if (! diagram.isNull()){
        auto flag=QMessageBox::question(this,tr("提示"),
                                        tr("是否确定关闭当前运行图文件？"));
        if (flag!=QMessageBox::Yes)
            return;
    }

    QString file=QFileDialog::getOpenFileName(this,tr("打开运行图"),
                                              {},
                                              "qETRC/pyETRC运行图文件 (*.pyetgr)\n"
        "JSON文件 (*.json)\n"
        "所有文件 (*)");
    if (file.isEmpty()) return;

    Diagram dia;
    bool succ=dia.fromJson(file);
    if(succ){
        diagram.clear();
        diagram=std::move(dia);
        emit diagramRefreshed();
    }else{
        QMessageBox::warning(this,tr("错误"),tr("无法打开运行图文件"));
    }
}

void AStartPage::actClear()
{
    auto flag=QMessageBox::question(this,tr("提示"),
                                    tr("是否确定关闭当前运行图文件？"));
    if (flag!=QMessageBox::Yes)
        return;
    diagram.clear();
    emit diagramRefreshed();
}

void AStartPage::refreshData()
{
    edDiaName->setText(diagram.filename());
}


#endif
