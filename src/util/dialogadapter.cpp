#include "dialogadapter.h"

#include <QtWidgets>

DialogAdapter::DialogAdapter(QWidget *widget_, QWidget *parent, bool closeButton):
    QDialog(parent), widget(widget_)
{
    setWindowTitle(widget->windowTitle());
    resize(widget->geometry().width(),widget->geometry().height());

    setAttribute(Qt::WA_DeleteOnClose);

    auto* vlay=new QVBoxLayout;
    vlay->addWidget(widget);

    if(closeButton){
        auto* btn=new QPushButton(tr("关闭"));
        connect(btn,SIGNAL(clicked()),this,SLOT(close()));
        vlay->addWidget(btn);
    }
    setLayout(vlay);
}
