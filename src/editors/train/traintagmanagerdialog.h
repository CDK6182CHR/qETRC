#pragma once

#include <QDialog>
#include <QAbstractTableModel>

class TrainTagManager;
class TrainCollection;
class TrainTag;

/**
 * 2025.08.14 For train tag manager. Currently we just use abstract model form.
 * The model actually maintains the list of the tags.
 */
class TrainTagManagerModel : public QAbstractTableModel
{
	Q_OBJECT;
	TrainTagManager& m_manager;
	QStringList m_tagNames;

public:

	enum Columns {
		ColName=0,
		ColNote,
		ColMAX
	};

	TrainTagManagerModel(TrainTagManager& manager, QObject* parent = nullptr);

	// 通过 QAbstractTableModel 继承
	int rowCount(const QModelIndex& parent) const override;
	int columnCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role)const override;

	std::shared_ptr<TrainTag> tagForRow(int row);

public slots:
	void refreshData();
};

class QTableView;


/**
 * TrainTag manager dialog for editing the train tags. 
 * For safety, the data edition is NOT allowed in the table, but only through outer operations.
 * For current version, we just reset the model each time the list is updated.
 */
class TrainTagManagerDialog : public QDialog
{
	Q_OBJECT;
	TrainCollection& m_coll;
	TrainTagManager& m_manager;
	TrainTagManagerModel* m_model;
	QTableView* m_table;

public:
	TrainTagManagerDialog(TrainCollection& coll, TrainTagManager& manager, QWidget* parent = nullptr);

private:
	void initUI();

	/**
	 * Return current selected tag in the table.
	 */
	std::shared_ptr<TrainTag> currentTag();

signals:

	// Train name and/or note changed
	void tagNameChanged(std::shared_ptr<TrainTag> tag, std::shared_ptr<TrainTag> data);

	// Only note changed (for this case, we need to do fewer things than nameChanged)
	void tagNoteChanged(std::shared_ptr<TrainTag> tag, std::shared_ptr<TrainTag> data);

	// New tag added
	void tagAdded(std::shared_ptr<TrainTag> tag);

	void removeTag(std::shared_ptr<TrainTag> tag);
	
public slots:
	void refreshData();

private slots:
	void actAdd();
	void actRemove();
	void actEdit();
	void actViewTrains();
};
