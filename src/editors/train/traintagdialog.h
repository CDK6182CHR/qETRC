#pragma once

#include <QDialog>
#include <QStandardItemModel>

class TrainTag;
class Train;
class TrainTagListDirectModel;

/**
 * Model for train tags that belonging to a single train. 
 */
class TrainTagModel: public QStandardItemModel
{
	Q_OBJECT;
public:
	enum Columns {
		ColName = 0,
		ColNote,
		ColMAX
	};

	TrainTagModel(QObject* parent = nullptr);
	void refreshData(const std::vector<std::shared_ptr<TrainTag>>& tags);


};

class QTableView;
class TrainTagManager;
class QLineEdit;

class TrainTagDialog : public QDialog
{
	Q_OBJECT;

	TrainTagManager& m_tagMan;
	std::shared_ptr<Train> m_train;
	QLineEdit* m_edNewTag;
	TrainTagModel* m_model;
	QTableView* m_table;

public:
	TrainTagDialog(TrainTagManager& tagman, TrainTagListDirectModel* completionModel, 
		std::shared_ptr<Train> train, QWidget* parent = nullptr);

	auto train() { return m_train; }

private:
	void initUI(TrainTagListDirectModel* completionModel);

signals:

	/**
	 * Add a tag to a train. We do not check if the tag already exists.
	 */
	void addTrainTag(const std::shared_ptr<Train>& train, const QString& tagName);

	void removeTrainTag(std::shared_ptr<Train> trrain, int index);

private slots:
	void actAddNewTag();
	void actDeleteTag();

public slots:
	void refreshData();
};
