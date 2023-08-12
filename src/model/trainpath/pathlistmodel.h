#pragma once
#include <QAbstractTableModel>
#include <memory>

#include "data/trainpath/trainpath.h"

class TrainPathCollection;
class TrainPath;

class PathListModel : public QAbstractTableModel 
{
	Q_OBJECT;
	TrainPathCollection& pathcoll;
public:
	enum {
		ColName=0,
		ColValid,
		ColStart,
		ColEnd,
		ColMAX
	};

	PathListModel(TrainPathCollection& pathcoll, QObject* parent = nullptr);

	// 通过 QAbstractTableModel 继承
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	//virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	//virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

public:

	/**
	 * The add/remove functions are responsible for performing REAL add/remove operations.
	 */
	void addPathAt(int index, std::unique_ptr<TrainPath>&& path);

	std::unique_ptr<TrainPath> popPathAt(int index);
};