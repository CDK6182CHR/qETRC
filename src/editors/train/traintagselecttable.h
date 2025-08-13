#pragma once

#include "util/qecontrolledtable.h"

class QEMoveableModel;
class TrainTagManager;

/**
 * 2025.08.13  Table for selecting the train tags, in train filter.
 * Similar to TrainNameRegexTable.
 */
class TrainTagSelectTable : public QEControlledTable
{
	Q_OBJECT;
	TrainTagManager& m_manager;
	QEMoveableModel* m_model;

public:
	TrainTagSelectTable(TrainTagManager& manager, QWidget* parent = nullptr);
	QVector<QString> selectedTags() const;

private:
	void initUI();

public slots:
	void refreshData(const QVector<QString>& tags);
};

