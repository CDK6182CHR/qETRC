#include "locatedialog.h"

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QRadioButton>
#include <QTimeEdit>
#include <QVBoxLayout>

#include <util/selectrailwaycombo.h>
#include "data/diagram/diagram.h"

#include <util/pagecomboforrail.h>


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
