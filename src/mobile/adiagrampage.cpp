#include "adiagrampage.h"

#ifdef QETRC_MOBILE
#include <QComboBox>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <kernel/diagramwidget.h>
#include <data/diagram/diagram.h>
#include <data/diagram/diagrampage.h>

ADiagramPage::ADiagramPage(Diagram &diagram, QWidget *parent):
    QWidget(parent), diagram(diagram)
{
    initUI();
}

void ADiagramPage::initUI()
{
    auto* vlay=new QVBoxLayout(this);

    cbPages=new QComboBox;
    vlay->addWidget(cbPages);

    stack=new QStackedWidget;
    vlay->addWidget(stack);

    connect(cbPages,qOverload<int>(&QComboBox::currentIndexChanged),
            stack,qOverload<int>(&QStackedWidget::setCurrentIndex));
}

void ADiagramPage::clearPages()
{
    while(!pages.empty()){
        auto p=pages.front();
        pages.pop_front();
        stack->removeWidget(p);
        p->setParent(nullptr);
        p->deleteLater();
    }
    cbPages->clear();
}

void ADiagramPage::refreshData()
{
    clearPages();

    if (diagram.pages().empty()){
        diagram.createDefaultPage();
    }

    foreach(auto page,diagram.pages()){
        auto* w=new DiagramWidget(diagram, page);
        stack->addWidget(w);
        pages.push_back(w);
        cbPages->addItem(page->name());
    }
}




#endif


