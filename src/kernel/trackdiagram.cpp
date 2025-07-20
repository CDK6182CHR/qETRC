#include "trackdiagram.h"
#include <QGraphicsSimpleTextItem>
#include <QGraphicsTextItem>
#include "data/train/train.h"
#include "data/diagram/trainline.h"
#include "data/train/traintype.h"
#include "data/diagram/diagramoptions.h"


TrackDiagram::TrackDiagram(TrackDiagramData &data, QWidget* parent):
    QGraphicsView(parent), _data(data)
{
	setScene(new QGraphicsScene(this));
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
	paintDiagram();
}

void TrackDiagram::refreshData()
{
	_data.refreshData();
    paintDiagram();
}

void TrackDiagram::repaintDiagram()
{
    paintDiagram();
}

void TrackDiagram::paintDiagram()
{
	scene()->clear();
	QColor gridColor(0, 255, 0);
	QPen defaultPen(gridColor, 1);
	QPen boldPen(gridColor, 2);
	double width = 24 * 3600 / seconds_per_pix;
	double height = _data.trackCount() * row_height;
	scene()->setSceneRect(0, 0, margins.left + margins.right + width,
		margins.top + margins.bottom + height);
	scene()->addRect(margins.left, margins.top, width, height, defaultPen);
	_initXAxis(width, height, gridColor);
	_initYAxis(width, defaultPen, boldPen);
	_addTrains();
}

void TrackDiagram::_initXAxis(double width, double height, const QColor& gridColor)
{
	QPen pen_half(gridColor, 1, Qt::DashLine),
		pen_hour(gridColor, 2),
		pen_other(gridColor, 1);
	scene()->addLine(margins.left - 15, 35, width + margins.left + 15, 35, pen_hour);
	scene()->addLine(margins.left - 15, margins.top + height + 15,
		width + margins.left + 15, margins.top + height + 15, pen_hour);
	for (int i = 0; i <= 24; i++) {
		double x = margins.left + i * 3600 / seconds_per_pix;
		int hour = i;
		auto* textItem = scene()->addSimpleText(QString::number(hour));
		textItem->setX(x - 12);
		textItem->setBrush(gridColor);
		textItem = scene()->addSimpleText(QString::number(hour));
		textItem->setX(x - 12);
		textItem->setY(scene()->height() - 30);
		textItem->setBrush(gridColor);

		if (i == 24) break;

		// 小时线 ..
		if (i != 0) {
			scene()->addLine(x, margins.top, x, margins.top + height, pen_hour);
		}
		for (int j = 1; j < 6; j++) {
			x += 10 * 60.0 / seconds_per_pix;
			auto* line = scene()->addLine(x, margins.top, x, margins.top + height);
			if (j * 10 == 30) {
				line->setPen(pen_half);
			}
			else {
				line->setPen(pen_other);
			}
		}

	}
}

void TrackDiagram::_initYAxis(double width, const QPen& defaultPen, const QPen& boldPen)
{
	Q_UNUSED(boldPen)
	int i = 0;
    foreach(const auto & track, _data.getTrackOrder()) {
		double y = margins.top + (i + 1) * row_height;
		scene()->addLine(margins.left, y, margins.left + width, y, defaultPen);
        auto* textItem = scene()->addSimpleText(track->name());
		textItem->setY(y - row_height / 2 - textItem->boundingRect().height() / 2);
		i++;
	}
}

void TrackDiagram::_addTrains()
{
	double start_y = margins.top;
    foreach(const auto & track, _data.getTrackOrder()) {
		if (track) {
			for (const auto& to : *track) {
				_addTrainRect(to, start_y);
			}
		}
		start_y += row_height;
	}
}

void TrackDiagram::_addTrainRect(const TrackOccupy& to, double start_y)
{
	auto [begTime, endTime] = to.occupiedRange(_data.diagramOptions().period_hours);
	double begin_x = _calXValue(begTime), end_x = _calXValue(endTime);
	auto train = to.item->line->train();
	const QColor& color = train->type()->pen().color();
	double width = end_x - begin_x;
	auto* rect = scene()->addRect(begin_x, start_y, width, row_height);
	rect->setBrush(color);
	rect->setToolTip(to.item->toString());
}

double TrackDiagram::_calXValue(const TrainTime& tm) const
{
	return margins.left + tm.secondsSinceStart() / seconds_per_pix;
}

void TrackDiagram::setXScale(int value)
{
	seconds_per_pix = value;
	paintDiagram();
}





