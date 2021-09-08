#include "detectroutingdialog.h"

#include "util/buttongroup.hpp"
#include "data/train/routing.h"
#include "data/train/traincollection.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QMessageBox>

DetectRoutingDialog::DetectRoutingDialog(TrainCollection &coll_,
                                         std::shared_ptr<Routing> original,
                                         bool fromContext_, QWidget *parent):
    QDialog(parent),coll(coll_),origin(original),fromContext(fromContext_)
{
    setWindowTitle(tr("车次识别"));
    resize(500,500);
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void DetectRoutingDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("此功能识别当前交路中的车次，将本运行图中能够搜索到的车次，"
        "设置为实体车次。建议仅识别完整车次，效率较高。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* flay=new QFormLayout;
    ckFullOnly=new QCheckBox(tr("仅识别完整车次"));
    ckFullOnly->setChecked(true);
    flay->addRow(tr("选项"),ckFullOnly);
    vlay->addLayout(flay);

    auto* g=new ButtonGroup<2>({"识别","取消"});
    g->connectAll(SIGNAL(clicked()),this,{SLOT(onApply()),SLOT(close())});
    vlay->addLayout(g);

    edOutput=new QTextBrowser;
    vlay->addWidget(edOutput);
    auto* btn=new QPushButton(tr("关闭"));
    connect(btn,&QPushButton::clicked,this,&QDialog::close);
    vlay->addWidget(btn);
}

void DetectRoutingDialog::onApply()
{
    auto tmp=std::make_shared<Routing>(*origin);
    QString report;
    const Routing* ignore=origin.get();
    bool flag=tmp->detectTrains(coll,ckFullOnly->isChecked(),report,ignore);
    QString text;
    if(flag){
        text.append(tr("交路在识别中改变。识别结果为\n%1"));
    }else{
        text.append(tr("交路未改变，序列仍为\n%1"));
    }
    text.append(tr("\n----------------\n详细报告：\n%2"));
    edOutput->setText(text.arg(tmp->orderString(),report));
    if(flag){
        QString text;
        if (fromContext) {
            text = tr("提示：当前操作由【交路上下文工具栏】调起，当前操作直接应用。"
                "如有问题，可以撤销。");
        }
        else {
            text = tr("提示：当前操作由【交路编辑停靠面板】调起，当前操作结果将填入表中，"
                "在停靠面板点击[确定]以应用结果。");
        }
        QMessageBox::information(this, tr("交路车次识别"), text);
        emit routingDetected(origin,tmp);
    }
}

BatchDetectRoutingDialog::BatchDetectRoutingDialog(TrainCollection& coll, QWidget* parent):
    QDialog(parent),coll(coll)
{
    setWindowTitle(tr("批量车次识别"));
    resize(500, 500);
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void BatchDetectRoutingDialog::initUI()
{
    auto* vlay = new QVBoxLayout(this);
    auto* lab = new QLabel(tr("此功能一次性对【所有交路】进行车次推定，尝试将交路中的虚拟车次"
        "识别为本运行图中的实际车次，同时检查实际车次有无错误。点击[识别]开始执行。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);
    auto* flay = new QFormLayout;
    ckFull = new QCheckBox(tr("仅识别完整车次"));
    ckFull->setChecked(true);
    flay->addRow(tr("选项"), ckFull);
    vlay->addLayout(flay);

    auto* g = new ButtonGroup<2>({ "解析","取消" });
    g->connectAll(SIGNAL(clicked()), this, { SLOT(actApply()),SLOT(close()) });
    vlay->addLayout(g);

    vlay->addWidget(new QLabel(tr("解析报告：")));
    edOutput = new QTextBrowser;
    vlay->addWidget(edOutput);
    auto* btn = new QPushButton(tr("关闭"));
    connect(btn, &QPushButton::clicked, this, &QDialog::close);
    vlay->addWidget(btn);
}

void BatchDetectRoutingDialog::actApply()
{
    QVector<int> indexes;
    QVector<std::shared_ptr<Routing>> routings;
    QString report;
    for (int i = 0; i < coll.routings().size(); i++) {
        auto r = coll.routings().at(i);
        auto rcp = std::make_shared<Routing>(*r);   // copy
        bool flag = rcp->detectTrains(coll, ckFull->isChecked(), report, r.get());
        if (flag) {
            rcp->updateTrainHooks();    //保证不能重复识别
            indexes.push_back(i);
            routings.push_back(rcp);
        }
    }
    edOutput->setText(report);
    if (!indexes.empty()) {
        QMessageBox::information(this, tr("提示"), tr("识别成功，影响到%1个交路。")
            .arg(indexes.size()));
        emit detectApplied(indexes, routings);
    }
    else {
        QMessageBox::information(this, tr("提示"), tr("没有交路受到影响。"));
    }
}
