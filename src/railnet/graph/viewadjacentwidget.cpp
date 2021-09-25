#include "adjacentlistwidget.h"
#include "vertexlistwidget.h"
#include "viewadjacentwidget.h"


ViewAdjacentWidget::ViewAdjacentWidget(const RailNet &net, QWidget *parent):
    QSplitter(parent),net(net)
{
    setWindowTitle(tr("邻接表浏览器"));
    resize(1000,800);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog);
    initUI();
}

void ViewAdjacentWidget::initUI()
{
    verWidget=new VertexListWidget(net);
    addWidget(verWidget);

    adjWidget=new AdjacentListWidget(net);
    addWidget(adjWidget);

    connect(verWidget,&VertexListWidget::currentVertexChanged,
            adjWidget,&AdjacentListWidget::setVertex);
}
