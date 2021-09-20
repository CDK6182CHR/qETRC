#include "locatedialog.h"

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QRadioButton>
#include <QTimeEdit>
#include <QVBoxLayout>
#include <QMessageBox>

#include <util/selectrailwaycombo.h>
#include "data/diagram/diagram.h"

#include <util/pagecomboforrail.h>
#include "data/diagram/stationbinding.h"


LocateDialog::LocateDialog(Diagram &diagram, QWidget *parent):
    QDialog(parent),diagram(diagram)
{
    setWindowTitle(tr("运行图定位"));
    initUI();
}

void LocateDialog::refreshData()
{
    cbRailway->refresh();
    onRailwayChanged(cbRailway->railway());
}

void LocateDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("此页面可根据所给时刻、线路及其里程（或车站）快速定位到"
        "运行图上。所选运行图窗口将被激活（activate）。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* flay=new QFormLayout;
    cbRailway=new SelectRailwayCombo(diagram.railCategory());
    flay->addRow(tr("线路"),cbRailway);
    connect(cbRailway,&SelectRailwayCombo::currentRailwayChanged,
            this,&LocateDialog::onRailwayChanged);

    auto* g=new QButtonGroup(this);
    rdStation=new QRadioButton(tr("车站"));
    rdStation->setChecked(true);
    connect(rdStation,&QRadioButton::toggled,this,&LocateDialog::onRdStationToggled);
    cbStation=new QComboBox;
    flay->addRow(rdStation,cbStation);

    g->addButton(rdStation);
    auto* rd=new QRadioButton(tr("里程标"));
    g->addButton(rd);
    spMile=new QDoubleSpinBox;
    spMile->setRange(-1000000,10000000);
    spMile->setSuffix(tr("  km"));
    spMile->setDecimals(3);
    spMile->setEnabled(false);
    flay->addRow(rd,spMile);

    edTime=new QTimeEdit;
    edTime->setDisplayFormat("hh:mm:ss");
    flay->addRow(tr("定位时刻"),edTime);

    cbPage=new PageComboForRail(diagram);
    flay->addRow(tr("运行图"),cbPage);
    vlay->addLayout(flay);

    auto* box=new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box,&QDialogButtonBox::accepted,this,&LocateDialog::onApplied);
    connect(box,&QDialogButtonBox::rejected,this,&QDialog::close);
    vlay->addWidget(box);
}

void LocateDialog::onRailwayChanged(std::shared_ptr<Railway> railway)
{
    cbStation->clear();
    cbPage->setRailway(railway);
    if(!railway) return;
    foreach(auto p,railway->stations()){
        QVariant v;
        v.setValue(p);
        cbStation->addItem(p->name.toSingleLiteral(),v);
    }
}

void LocateDialog::onRdStationToggled(bool on)
{
    cbStation->setEnabled(on);
    spMile->setEnabled(!on);
}

void LocateDialog::onApplied()
{
    auto railway=cbRailway->railway();
    if(!railway)return;
    if (rdStation->isChecked()){
        //if(cbStation->currentIndex()==-1) return;
        //如果是-1，交给mainwindow报错
        auto st=qvariant_cast<std::shared_ptr<RailStation>>(cbStation->currentData());
        emit locateOnStation(cbPage->pageIndex(),railway,st,edTime->time());
    }else{
        emit locateOnMile(cbPage->pageIndex(),railway,spMile->value(),edTime->time());
    }
    done(Accepted);
}

void LocateDialog::showDialog()
{
    refreshData();
    show();
}

LocateBoundingDialog::LocateBoundingDialog(Diagram &diagram, QWidget *parent):
    QDialog(parent), diagram(diagram)
{
    setWindowTitle(tr("列车车站定位"));
    initUI();
}

void LocateBoundingDialog::showForStation(const QVector<TrainStationBounding> &boudingList_,
                                          const QTime &tm)
{
    this->boudingList=boudingList_;
    // 先把无效的弄掉
    cbBound->clear();
    for(auto p=boudingList.begin();p!=boudingList.end();){
        if(p->line.expired() || p->railStation.expired()){
            p=boudingList.erase(p);
        }else{
            cbBound->addItem(tr("%1 @ %2")
           .arg(p->railStation.lock()->name.toSingleLiteral(),
                p->line.lock()->railway()->name() ));
            ++p;
        }
    }

    if (boudingList.empty()){
        QMessageBox::warning(this,tr("错误"),
                             tr("当前没有可用的铺画点数据，可能因为数据更改。"
                "请考虑执行刷新操作。"));
        return;
    }
    edTime->setTime(tm);
    show();
}

void LocateBoundingDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("当前车站可能有多个铺画点，或显示在多张运行图中。"
        "请选择要定位的具体铺画点和运行图。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);
    auto* flay=new QFormLayout;
    cbBound=new QComboBox;
    flay->addRow(tr("铺画点选择"),cbBound);
    connect(cbBound, qOverload<int>(&QComboBox::currentIndexChanged),
        this, qOverload<int>(&LocateBoundingDialog::onBoundComboChanged));
    cbPage=new PageComboForRail(diagram);
    flay->addRow(tr("运行图页面"),cbPage);
    edTime=new QTimeEdit;
    flay->addRow(tr("定位时刻"),edTime);
    vlay->addLayout(flay);

    auto* box=new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    vlay->addWidget(box);

    connect(box,&QDialogButtonBox::accepted,
            this,&LocateBoundingDialog::onApply);
    connect(box,&QDialogButtonBox::rejected,
            this,&LocateBoundingDialog::close);
}

void LocateBoundingDialog::onApply()
{
    int bound_index=cbBound->currentIndex();
    if(bound_index==-1) return;  // 这个一般不会出现
    const auto& bound=boudingList.at(bound_index);
    if (bound.line.expired() || bound.railStation.expired()){
        QMessageBox::warning(this,tr("错误"),
                             tr("所选数据无效，可能运行图数据发生了变更。"
                "请考虑执行刷新操作。"));
        return;
    }
    auto rail=bound.line.lock()->railway();
    int page_index=cbPage->pageIndex();
    if (page_index == -1) {
        QMessageBox::warning(this, tr("错误"),
            tr("所选数据无效，可能运行图数据发生了变更。"
                "请考虑执行刷新操作。"));
        return;
    }
        
    emit locateOnStation(page_index, rail, bound.railStation.lock(),
                         edTime->time());
    done(Accepted);
}

void LocateBoundingDialog::onBoundComboChanged(int i)
{
    if(i==-1) return;
    const auto& item=boudingList.at(i);
    if (item.line.expired()){
        QMessageBox::warning(this,tr("错误"),
                             tr("数据已失效，无法定位到指定线路。可能因为线路数据改变。"));
    }else{
        cbPage->setRailway(item.line.lock()->railway());
    }
}
