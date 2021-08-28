#include "batchparseroutingdialog.h"

#include <model/train/routinglistmodel.h>
#include <QtWidgets>
#include "util/buttongroup.hpp"
#include "data/common/qesystem.h"
#include "data/train/routing.h"
#include "data/train/traincollection.h"
#include "util/dialogadapter.h"

BatchParseRoutingDialog::BatchParseRoutingDialog(TrainCollection &coll_, QWidget *parent):
    QDialog(parent),coll(coll_),model(new RoutingListModel(this))
{
    setWindowTitle(tr("批量交路解析"));
    resize(1200,800);
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void BatchParseRoutingDialog::initUI()
{
    auto* hlay=new QHBoxLayout(this);

    auto* vlay=new QVBoxLayout;
    auto* lab=new QLabel(tr("此功能批量解析车次套用顺序字符串。每行一个交路的信息，交路第一个车次"
        "将被用作默认的交路名称。可以指定车次分隔符，或采用内置的分隔符：{ -,~,—,～ }。"
        "但每个交路只能采用一种分隔符，不能混用。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);
    auto* flay=new QFormLayout;
    edSplitter=new QLineEdit;
    flay->addRow(tr("分隔符"),edSplitter);
    auto* ch=new QHBoxLayout;
    ckVirtual=new QCheckBox(tr("保留纯虚交路"));
    ch->addWidget(ckVirtual);
    ckFull=new QCheckBox(tr("仅识别完整车次"));
    ckFull->setChecked(true);
    ch->addWidget(ckFull);
    flay->addRow(tr("选项"),ch);
    vlay->addLayout(flay);

    edText=new QTextEdit;
    vlay->addWidget(edText);
    hlay->addLayout(vlay);

    auto* g=new ButtonGroup<3,QVBoxLayout>({"解析","信息","关闭"});
    g->setMinimumWidth(80);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actParse()),SLOT(actDetail()),SLOT(close())});
    hlay->addLayout(g);

    table=new QTableView;
    table->setModel(model);
    int c = 0;
    for (int w : {120, 40, 40, 400}) {
        table->setColumnWidth(c++, w);
    }

    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    hlay->addWidget(table);
}

void BatchParseRoutingDialog::actParse()
{
    const QString& text = edText->toPlainText();
    if (text.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("空输入"));
        return;
    }
    auto sp = edSplitter->text().simplified();
    report.clear();

    QSet<QString> names;   //用来避免现在添加的这一批名称重复

    QList<std::shared_ptr<Routing>> routings;
    auto s = text.splitRef("\n");
    foreach(const auto & _line, s) {
        QString line = _line.toString().simplified();
        if (line.isEmpty())continue;
        auto r = std::make_shared<Routing>();
        r->parse(coll, line, sp, ckFull->isChecked(), report, nullptr);
        if (r->empty())
            continue;   //总是拒绝完全空的
        if (ckVirtual->isChecked() || r->anyValidTrains()) {
            // 此交路合理，进行添加
            QString name = coll.validRoutingName(r->order().front().name());
            while (names.contains(name)) {
                name.append("_");
            }
            r->setName(name);
            names.insert(name);
            r->updateTrainHooks();    // 更新hook，以保证后面的车次不能重复进来
            routings.push_back(r);
        }
    }
    if (!routings.isEmpty()) {
        model->setRoutings(routings);
        emit routingsParsed(routings);
        QMessageBox::information(this, tr("提示"), tr("成功解析%1个新交路数据，数据已经直接提交。"
            "可在[信息]中查看解析的详细报告信息。")
            .arg(routings.size()));
    }
    else {
        QMessageBox::information(this, tr("提示"), tr("没有新交路被解析。"));
    }

}

void BatchParseRoutingDialog::actDetail()
{
    auto* t = new QTextBrowser();
    t->setText(report);
    auto* a = new DialogAdapter(t, this);
    a->show();
}
