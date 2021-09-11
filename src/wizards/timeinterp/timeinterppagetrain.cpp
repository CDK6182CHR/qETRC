#include "timeinterppagetrain.h"

#include <model/train/trainlistreadmodel.h>
#include "data/diagram/diagram.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QMessageBox>
#include <util/railrulercombo.h>
#include "data/common/qesystem.h"
#include "util/utilfunc.h"

TimeInterpPageTrain::TimeInterpPageTrain(Diagram &diagram, QWidget *parent):
    QWizardPage(parent), diagram(diagram),
    model(new TrainListReadModel(diagram.trainCollection().trains(),this))
{
    setTitle(tr("选项"));
    initUI();
}

QVector<std::shared_ptr<Train> > TimeInterpPageTrain::selectedTrains()
{
    QVector<std::shared_ptr<Train>> trains;
    auto sel=table->selectionModel()->selectedRows();
    std::sort(sel.begin(),sel.end(),qeutil::ltIndexRow);
    foreach(const auto& idx,sel){
        trains.push_back(model->trains().at(idx.row()));
    }
    return trains;
}

void TimeInterpPageTrain::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* hlay=new QHBoxLayout;
    auto* lab=new QLabel(tr("此功能按标尺推定未给出时刻的通过站的时刻。可以选择是否推定到线路的起点或终点。"
        "与pyETRC不同，这里不会删除列车的非铺画站点，但如果有其他干扰车站的存在，可能会使得结果非预期。"));
    lab->setWordWrap(true);
    hlay->addWidget(lab);
    auto* btn=new QPushButton(tr("逻辑说明"));
    //btn->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    btn->setMaximumWidth(150);
    hlay->addWidget(btn);
    connect(btn,&QPushButton::clicked,this,&TimeInterpPageTrain::actHelp);
    vlay->addLayout(hlay);

    auto* flay=new QFormLayout;
    cbRuler=new RailRulerCombo(diagram.railCategory());
    flay->addRow(tr("线路、标尺"),cbRuler);

    hlay=new QHBoxLayout;
    ckToStart=new QCheckBox(tr("推定到本线起点"));
    hlay->addWidget(ckToStart);
    ckToEnd=new QCheckBox(tr("推定到本线终点"));
    hlay->addWidget(ckToEnd);
    flay->addRow(tr("外插选项"),hlay);

    cbPrec=new QComboBox;
    for(int i:{1,5,10,30})
        cbPrec->addItem(tr("%1秒").arg(i),i);
    cbPrec->addItem(tr("1分钟"),60);
    cbPrec->setMaximumWidth(200);
    flay->addRow(tr("时刻粒度"),cbPrec);
    vlay->addLayout(flay);
    vlay->addWidget(new QLabel(tr("请在下表中选择要推定的车次，可多选。")));

    table=new QTableView;
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->setSelectionBehavior(QTableView::SelectRows);
    table->setSelectionMode(QTableView::MultiSelection);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->resizeColumnsToContents();
    vlay->addWidget(table);
}

void TimeInterpPageTrain::actHelp()
{
    QString text=QObject::tr("此功能按将区间标尺纯运行时分（已经扣去起停附加时分）的比例，将实际纯运行时分"
              "（扣去起停附加时分）分配到各个子区间上。\n"
              "若选择“推定到本线起点”，则将本线第一个站点（不分上下行）到列车最靠近该站点的区间内各个站点"
              "时刻使用标尺计算，“推定到本线终点”同理。"
              "选择“下一步”后，程序计算出各车次相对时刻与标尺的相对误差，计算规则是各个【已知区间】"
              "标尺运行时分（考虑起停）和已知运行时分之差值的绝对值之和与该区间【已知运行时分】之比。"
              "\n各行背景色深浅表示相对误差大小，若相对误差大则不推荐继续推定时刻。");
    QMessageBox::information(this,tr("逻辑说明"),text);
}

bool TimeInterpPageTrain::validatePage()
{
    if(!cbRuler->ruler()){
        QMessageBox::warning(this,tr("错误"),tr("请选择标尺。\n"
        "出现这个错误的原因可能是当前所选线路没有可用标尺。"));
        return false;
    }
    if(table->selectionModel()->selectedRows().isEmpty()){
        QMessageBox::warning(this,tr("错误"),tr("请至少选择一个车次。"));
        return false;
    }
    return true;
}
