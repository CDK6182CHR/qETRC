#include "railrulercombo.h"

#include <QComboBox>
#include "data/rail/railcategory.h"
#include "data/rail/rail.h"

RailRulerCombo::RailRulerCombo(RailCategory &cat_, QWidget *parent):
    QHBoxLayout(parent),cat(cat_)
{
    initUI();
}

void RailRulerCombo::initUI()
{
    cbRail=new QComboBox;
    cbRuler=new QComboBox;
    cbRail->setEditable(false);
    cbRuler->setEditable(false);
    addWidget(cbRail);
    addWidget(cbRuler);

    connect(cbRail,SIGNAL(currentIndexChanged(int)),this,
            SLOT(onRailIndexChanged(int)));
    connect(cbRuler,SIGNAL(currentIndexChanged(int)),this,
            SLOT(onRulerIndexChanged(int)));

    refreshRailwayList();
}

void RailRulerCombo::onRailIndexChanged(int i)
{
    _railway.reset();
    _ruler.reset();
    if (i>=0 && i<cat.railways().size()){
        _railway=cat.railways().at(i);
        cbRuler->clear();
        for(auto p:_railway->rulers()){
            cbRuler->addItem(p->name());
        }
        emit railwayChagned(_railway);
    }
    else{
//        qDebug()<<"RailRulerCombo::onRailIndexChanged: WARNING: "<<
//                  "Invalid index: "<<i<<Qt::endl;
    }
}

void RailRulerCombo::onRulerIndexChanged(int i)
{
    _ruler.reset();
    if(_railway){
        if(i>=0&&i<_railway->rulers().size()){
            _ruler=_railway->getRuler(i);
            emit rulerChanged(_ruler);
        }
    }
}

void RailRulerCombo::refreshRulerList()
{
    cbRuler->clear();
    if(_railway){
        for(auto p:_railway->rulers()){
            cbRuler->addItem(p->name());
        }
    }
}

void RailRulerCombo::refreshRailwayList()
{
    cbRail->clear();
    for(auto p:cat.railways()){
        cbRail->addItem(p->name());
    }
}
