#include "adiagrampage.h"

#ifdef QETRC_MOBILE
#include <QComboBox>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QMessageBox>
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

    auto* hlay=new QHBoxLayout;
    cbPages=new QComboBox;
    hlay->addWidget(cbPages);

    auto* btn=new QPushButton(tr("转到车次"));
    hlay->addWidget(btn);
    vlay->addLayout(hlay);
    connect(btn,&QPushButton::clicked,this,&ADiagramPage::actToTrain);

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

DiagramWidget *ADiagramPage::currentPage()
{
    return static_cast<DiagramWidget*>(stack->currentWidget());
}

void ADiagramPage::refreshData()
{
    clearPages();

    if (diagram.pages().empty() && ! diagram.isNull()){
        diagram.createDefaultPage();
    }

    foreach(auto page,diagram.pages()){
        auto* w=new DiagramWidget(diagram, page);
        stack->addWidget(w);
        pages.push_back(w);
        cbPages->addItem(page->name());
    }
}

void ADiagramPage::actToTrain()
{
    auto* w=currentPage();
    if (!w){
        QMessageBox::warning(this,tr("错误"),tr("没有打开的运行图"));
        return;
    }
    auto t=w->selectedTrain();
    if(!t){
        QMessageBox::warning(this,tr("错误"),tr("没有选中的车次！"));
    }
    emit switchToTrain(t);
}




#endif


