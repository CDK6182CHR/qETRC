#pragma once

#include <QStandardItemModel>
#include <QDialog>

class TrainCollection;
struct DiagramOptions;
class QTableView;
class TrainFilterSelectorCore;
class TrainFilterSelector;

/**
 * 2026.02.17  Similar to TrainListStdModel, but contains more information.
 * Mainly provided for output.
 */
class TrainInfoSummaryModel: public QStandardItemModel 
{
	Q_OBJECT;
	
	const DiagramOptions& m_ops;
	TrainCollection& m_coll;
	const TrainFilterSelectorCore& m_filterCore;

public:

	enum Columns {
		ColTrainName = 0,
		ColStarting,
		ColTerminal,
		ColType,
		ColPaintedMile,
		ColPaintedTime,
		ColPaintedTravSpeed,
		ColPaintedTechSpeed,
		ColTotalTime,
		ColTotalRunTime,
		ColTotalStayTime,
		ColTotalMile,
		ColTotalTravSpeed,
		ColTotalTechSpeed,
		ColRouting,
		ColCarriage,
		ColOwner,
		ColMAX
	};

	TrainInfoSummaryModel(const DiagramOptions& m_ops, TrainCollection& coll, 
		const TrainFilterSelectorCore& filterCore, QObject* parent = nullptr);

	void setupModel();

public slots:
	void refreshData();
};

/**
 * 2026.02.17  Train summary dialog, containing basic statistical data of trains.
 * Similar to TrainListWidget.
 */
class TrainSummaryDialog : public QDialog
{
	Q_OBJECT;
	const DiagramOptions& m_ops;
	TrainCollection& m_coll;
	TrainFilterSelector* m_filter;

	TrainInfoSummaryModel* m_model;
	QTableView* m_table;

public:
	TrainSummaryDialog(const DiagramOptions& m_ops, TrainCollection& coll, QWidget* parent = nullptr);

private:
	void initUI();

private slots:
	void exportCsv();
	void refreshData();
};
