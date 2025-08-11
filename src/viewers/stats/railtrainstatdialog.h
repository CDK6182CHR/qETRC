#pragma once

#include <memory>
#include <QDialog>
#include <QStandardItemModel>

class Railway;
class Train;
class TrainCollection;
struct DiagramOptions;
class QTableView;
class TrainFilterSelector;
class TrainFilterSelectorCore;
class QDoubleSpinBox;

/**
 * 2025.08.11  Statistical information for trains for given railway. Use standard item model style.
 */
class RailTrainStatModel : public QStandardItemModel
{
	Q_OBJECT;

	double m_min_mile = 0;   // Minimal mile for displaying trains
	std::shared_ptr<Railway> m_railway;
	const TrainFilterSelectorCore& m_filter_core;
	TrainCollection& m_coll;
	DiagramOptions& m_options;

public:
	enum Columns {
		ColTrainName = 0,
		ColStarting,
		ColTerminal,
		ColType,
		ColLineCount,
		ColStationCount,
		ColMile,
		ColTotalTime,
		ColRunningTime,
		ColStopTime,
		ColTravSpeed,
		ColTechSpeed,
		ColMAX
	};

	RailTrainStatModel(std::shared_ptr<Railway> railway, const TrainFilterSelectorCore& filter_core, 
		TrainCollection& coll, DiagramOptions& options, QObject* parent = nullptr);

private:
	void setupModelData();

public slots:
	void refreshData();
	void setMinMile(double mile);
};


class RailTrainStatDialog : public QDialog
{
	Q_OBJECT;
	std::shared_ptr<Railway> m_railway;
	TrainCollection& m_coll;
	DiagramOptions& m_options;
	RailTrainStatModel* m_model;

	QDoubleSpinBox* m_spMinMile;
	TrainFilterSelector* m_filter;
	QTableView* m_table;

public:
	RailTrainStatDialog(std::shared_ptr<Railway> railway, TrainCollection& coll,
		DiagramOptions& options, QWidget* parent = nullptr);

private:
	void initUI();

public slots:
	void refreshData();

private slots:
	void actExportCsv();
};
