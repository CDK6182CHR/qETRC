#include "selectrailwaycombo.h"


SelectRailwayCombo::SelectRailwayCombo(RailCategory &cat_, QWidget *parent):
    QComboBox(parent),cat(cat_)
{
    connect(this,SIGNAL(currentIndexChanged(int)),this,SLOT(onIndexChanged(int)));
    refresh();
}

void SelectRailwayCombo::refresh()
{
    clear();
    for(auto p:cat.railways()){
        addItem(p->name());
    }
}

void SelectRailwayCombo::onIndexChanged(int i)
{
    if(0<=i && i<cat.railways().size()){
        _railway=cat.railways().at(i);
        emit currentRailwayChanged(_railway);
    }else{
        _railway.reset();
    }
}
