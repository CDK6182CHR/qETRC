#include "intervaltraindialog.h"

#include <dialogs/trainfilter.h>
#include <data/diagram/diagram.h>
#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>
#include <data/common/qesystem.h>
#include <util/buttongroup.hpp>
#include <data/train/traintype.h>
#include <util/utilfunc.h>

#include "intervaltraintable.h"



IntervalTrainDialog::IntervalTrainDialog(Diagram &diagram, QWidget *parent):
    QDialog(parent),
    diagram(diagram),filter(new TrainFilter(diagram,this)),
    counter(diagram.trainCollection(),filter->getCore()),
    table(new IntervalTrainTable(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void IntervalTrainDialog::initUI()
{
    setWindowTitle(tr("区间车次表"));
    resize(800, 800);
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;

    edFrom=new QLineEdit;
    edTo=new QLineEdit;
    flay->addRow(tr("发站"),edFrom);
    flay->addRow(tr("到站"),edTo);

    auto* hlay=new QHBoxLayout;
    ckStop=new QCheckBox(tr("仅停车（含始发终到）车次"));
    ckBusiness=new QCheckBox("仅营业车次");
    hlay->addWidget(ckBusiness);
    hlay->addWidget(ckStop);

    auto* btn=new QPushButton(tr("车次筛选器"));
    hlay->addWidget(btn);
    connect(btn,&QPushButton::clicked,filter,&TrainFilter::show);

    flay->addRow(tr("筛选"),hlay);
    vlay->addLayout(flay);

    auto* g=new ButtonGroup<3>({"查询","导出CSV","关闭"});
    g->connectAll(SIGNAL(clicked()),this,
                  {SLOT(updateData()),SLOT(toCsv()),SLOT(close())});
    vlay->addLayout(g);



    vlay->addWidget(table);
}

void IntervalTrainDialog::updateData()
{
    const QString& from_name=edFrom->text();
    const QString& to_name=edTo->text();
    if(from_name.isEmpty() || to_name.isEmpty()){
        QMessageBox::warning(this,tr("错误"),tr("请输入非空车站名！"));
        return;
    }
    counter.setBusinessOnly(ckBusiness->isChecked());
    counter.setStopOnly(ckStop->isChecked());

    auto&& data=counter.getIntervalTrains(from_name,to_name);
    table->getModel()->resetData(std::move(data));
    table->getModel()->refreshData();
}

void IntervalTrainDialog::toCsv()
{
    qeutil::exportTableToCsv(table->getModel(), this,
                             tr("区间车次表%1-%2").arg(edFrom->text(),
                                                  edTo->text()));
}
