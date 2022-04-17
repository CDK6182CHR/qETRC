#include "railstationcombo.h"

#include <QComboBox>
#include <data/rail/railcategory.h>
#include <data/rail/railway.h>

RailStationCombo::RailStationCombo(RailCategory &railcat, QWidget *parent):
    QHBoxLayout(parent), _railcat(railcat)
{
    initUI();
}

void RailStationCombo::initUI()
{
    cbRail=new QComboBox;
    addWidget(cbRail);
    connect(cbRail,qOverload<int>(&QComboBox::currentIndexChanged),
            this,&RailStationCombo::onRailwayChanged);
    cbStation=new QComboBox;
    addWidget(cbStation);
    connect(cbStation, qOverload<int>(&QComboBox::currentIndexChanged),
            this,&RailStationCombo::onStationChanged);
    refreshData();
}

void RailStationCombo::onRailwayChanged(int i)
{
    if (i==-1){
        _railway.reset();
    }else{
        auto tmp=_railcat.railways().at(i);
        if (tmp!=_railway){
            _railway=tmp;
            setupStationCombo();
            emit railwayChanged(_railway);
        }
    }
}

void RailStationCombo::onStationChanged(int i)
{
    if (i==-1){
        _station.reset();
    }else{
        auto tmp=_railway->stations().at(i);
        if(tmp!=_station){
            _station=tmp;
            emit stationChanged(_station);
        }
    }
}

void RailStationCombo::setupStationCombo()
{
    cbStation->clear();
    if(!_railway) return;
    foreach(auto st,_railway->stations()){
        cbStation->addItem(st->name.toSingleLiteral());
    }
}

void RailStationCombo::refreshData()
{
    cbRail->clear();
    foreach(auto rail,_railcat.railways()){
        cbRail->addItem(rail->name());
    }
    // 此操作自动调起cbStation的操作
}
