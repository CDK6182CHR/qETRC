#include "arailanalysis.h"
#include "arailpage.h"

#include <QComboBox>
#include <QVBoxLayout>

#include <util/selectrailwaycombo.h>
#include <data/diagram/diagram.h>
#include <editors/forbidwidget.h>
#include <editors/railstationwidget.h>

#ifdef QETRC_MOBILE

ARailPage::ARailPage(Diagram &diagram, QWidget *parent):
    QWidget(parent), diagram(diagram)
{
    initUI();
}

void ARailPage::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    cbRails=new SelectRailwayCombo(diagram.railCategory());
    vlay->addWidget(cbRails);

    tab=new QTabWidget;
    tab->setTabPosition(QTabWidget::East);

    railWidget=new RailStationWidget(diagram.railCategory(),true);
    railWidget->setReadOnly();
    tab->addTab(railWidget,tr("基线"));

    connect(cbRails,&SelectRailwayCombo::currentRailwayChanged,
            this,&ARailPage::setRailway);

    anaWidget = new ARailAnalysis(diagram,cbRails);
    tab->addTab(anaWidget,tr("分析"));

    vlay->addWidget(tab);
}

void ARailPage::setRailway(std::shared_ptr<Railway> railway)
{
    railWidget->setRailway(railway);
}

void ARailPage::refreshData()
{
    cbRails->refresh();
    railWidget->refreshData();
}





#endif
