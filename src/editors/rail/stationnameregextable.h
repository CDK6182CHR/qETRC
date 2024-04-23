#pragma once

#include "util/qecontrolledtable.h"
#include <QRegularExpression>
#include <memory>
#include <QVector>

class RailCategory;
class QEMoveableModel;

/**
 * 2024.04.23  Similar to TrainNameRegexTable, used in train filter (for starting and terminal stations)
 */
class StationNameRegexTable: public QEControlledTable
{
	Q_OBJECT;
	//std::shared_ptr<RailCategory> railcat;
	QEMoveableModel* model;

public:
	StationNameRegexTable(QWidget* parent = nullptr);
	QVector<QRegularExpression> names()const;

private:
	void initUI();

public slots:
	void refreshData(const QVector<QRegularExpression>& data);
	void clearNames();
};
