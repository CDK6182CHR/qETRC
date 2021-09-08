#include "parseroutingdialog.h"

#include "data/train/routing.h"
#include "util/buttongroup.hpp"

#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QTextBrowser>
#include <QTextEdit>

ParseRoutingDialog::ParseRoutingDialog(TrainCollection &coll_,
                                       bool fromContext_,  std::shared_ptr<Routing> original,
                                       QWidget *parent):
    QDialog(parent), coll(coll_), fromContext(fromContext_), origin(original)
{
    resize(500,500);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("交路文本解析"));
    initUI();
}

void ParseRoutingDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("此功能允许一次解析一列车次的套跑交路，可指定车次的分隔符；如果"
        "不指定，则使用内置的分隔符，内置分隔符包括：{ -,~,—,～ }。"
        "分隔符不能混用，只能采用一种。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* flay=new QFormLayout;
    edSplitter=new QLineEdit;
    flay->addRow(tr("分隔符"),edSplitter);
    ckFullOnly=new QCheckBox(tr("仅识别完整车次"));
    ckFullOnly->setChecked(true);
    flay->addRow(tr("选项"),ckFullOnly);
    vlay->addLayout(flay);
    vlay->addWidget(new QLabel("套跑交路文本："));

    edText=new QTextEdit;
    edText->setMaximumHeight(50);
    vlay->addWidget(edText);

    if(fromContext){
        lab=new QLabel(tr("【注意】当前对话框从【交路上下文工具栏】中调起，"
        "现在解析的交路将直接覆盖原有交路的全部信息。如果想解析的序列附加到既有序列，"
        "请从交路编辑的停靠面板中调用本功能。"));
    }else{
        lab=new QLabel(tr("【注意】当前对话框从【交路编辑停靠面板】中调起，"
        "现在解析的交路将被附加到既有的序列后，点击[确定]提交更改。如果想要当前解析的序列直接覆盖既有序列，"
        "请从交路上下文工具栏中调用本功能。"));
    }
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* g=new ButtonGroup<2>({"解析","取消"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actApply()),SLOT(close())});

    vlay->addWidget(new QLabel(tr("解析结果：")));
    edOutput=new QTextBrowser;
    vlay->addWidget(edOutput);
    auto* btn=new QPushButton("关闭");
    connect(btn,&QPushButton::clicked,this,&QDialog::close);
    vlay->addWidget(btn);

}

void ParseRoutingDialog::actApply()
{
    auto&& sp=edSplitter->text();
    auto&& text=edText->toPlainText();

    if(text.isEmpty()){
        QMessageBox::warning(this,tr("错误"),tr("空序列"));
        return;
    }

    QString report;
    auto t=origin->copyBase();
    Routing* ignore;
    if(fromContext){
        ignore=origin.get();
    }else{
        ignore=nullptr;
    }
    t->parse(coll,text,sp,ckFullOnly->isChecked(),report,ignore);

    edOutput->setPlainText(tr("%1\n-----------解析报告-----------\n%2")
                           .arg(t->orderString(),report));
    emit routingParsed(origin,t);
}
