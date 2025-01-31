#pragma once

#include <QWidget>
#include <memory>

class QTableView;
class TrainListStdModel;
class TrainCollection;
class Train;

/**
 * 2025.01.31  Widget for selecting a series of ordered trains, mainly a controlled table widget.
 * The trains are held in TrainListStdModel.
 */
class SelectTrainWidget : public QWidget
{
	Q_OBJECT;
	TrainCollection& m_coll;
	TrainListStdModel* m_model;
	QTableView* m_table;

public:
	SelectTrainWidget(TrainCollection& coll, QWidget* parent = nullptr);

	std::vector<std::shared_ptr<Train>> trains()const;

private:
	void initUI();

private slots:
	void actAdd();
	void actRemove();
	void actMoveUp();
	void actMoveDown();
};