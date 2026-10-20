#include "trainpolylineitem.h"

#include <QGraphicsPathItem>
#include "trainpolylinebuilder.h"
#include "data/diagram/config.h"

TrainPolyLineItem::TrainPolyLineItem(QGraphicsItem* parent):
	QGraphicsItem(parent)
{
}

void TrainPolyLineItem::buildItem(TrainPolyLineBuilder& builder)
{
    m_pathItem = new QGraphicsPathItem(builder.path().path(), this);
    m_pathItem->setPen(builder.style().line_pen());

    if (builder.config().valid_width > 1) {
        QPen expen(Qt::transparent, builder.style().line_pen().width() * builder.config().valid_width);
        m_expandItem = new QGraphicsPathItem(builder.path().path(), this);
        m_expandItem->setPen(expen);
    }

    // Draw the markers
    foreach(const auto& minfo, builder.markerInfos()) {
        auto* p = builder.markerBuilder()->build_marker(minfo, builder.style().marker().marker_size(), 
            builder.style().marker().pen(), this);
        if (p) {
            m_markers.push_back(p);
        }
    }
}

QRectF TrainPolyLineItem::boundingRect() const
{
    return QRectF();
}

void TrainPolyLineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}
