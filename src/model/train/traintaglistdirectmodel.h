#pragma once

#include <QAbstractListModel>
#include <QList>

class TrainTagManager;

/**
 * 2025.08.14  "Direct" model (using abstract model APIs) for train tag list.
 * The underlying data is just a list of string, representing tag names.
 * When the data is imported from TrainTagManager, the data is sorted.
 */
class TrainTagListDirectModel : public QAbstractListModel
{
	Q_OBJECT;
	QStringList m_tags;

public:
	using QAbstractListModel::QAbstractListModel;

	void refreshData(const TrainTagManager& manager);

	void clear();

public slots:
	void setTags(QStringList tags);

	// 通过 QAbstractListModel 继承
	int rowCount(const QModelIndex& parent) const override;

	QVariant data(const QModelIndex& index, int role) const override;

};
