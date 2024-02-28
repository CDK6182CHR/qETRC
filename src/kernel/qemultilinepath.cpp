#include "qemultilinepath.h"

QEMultiLinePath::QEMultiLinePath(const QPointF& p):
	_path(p), _isNewPath(true)
{
}

void QEMultiLinePath::moveTo(const QPointF& p)
{
	_path.moveTo(p);
	_isNewPath = true;
}

void QEMultiLinePath::moveTo(qreal x, qreal y)
{
	_path.moveTo(x, y);
	_isNewPath = true;
}

void QEMultiLinePath::lineTo(const QPointF& p)
{
	if (_isNewPath) {
		_path.lineTo(p);
		_isNewPath = false;
	}
	else {
		_path.moveTo(_path.currentPosition());
		_path.lineTo(p);
	}
}

void QEMultiLinePath::lineTo(qreal x, qreal y)
{
	lineTo({ x,y });
}

void QEMultiLinePath::addPolygon(const QPolygonF& p)
{
	_path.addPolygon(p);
	_isNewPath = false;
}

void QEMultiLinePath::closeSubPath()
{
	_path.closeSubpath();
	_isNewPath = false;
}
