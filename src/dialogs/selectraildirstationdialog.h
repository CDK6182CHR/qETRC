#pragma once

#include <memory>
#include <QDialog>

#include "util/buttongroup.hpp"
#include "data/common/direction.h"

class RailStationModel;
class QTableView;
class SelectRailwayCombo;
class RailCategory;
class RailStation;
class Railway;

/**
 * Dialog for selecting railway stations for selected direction (up/down).
 * Designed mainly for importing railway stations to train timetables (on editing).
 * Different from SelectRailStationDialog: current class allows selecting railway and direction.
 * This is almost the same as what presented in RulerPaintPageStation.
 */
class SelectRailDirStationDialog : public QDialog
{
	Q_OBJECT
	RailCategory& m_cat;
	SelectRailwayCombo* m_cbRail;
	RailStationModel* m_model;
	QTableView* m_table;
	RadioButtonGroup<2>* m_radioDireciton;

public:
	SelectRailDirStationDialog(RailCategory& cat, const QString& prompt = {}, QWidget * parent = nullptr);

	/**
	 * Returns the list of currently selected stations.
	 */
	QList<std::shared_ptr<const RailStation>> selectedStations(); 

	struct Result {
		bool valid;
		std::shared_ptr<const Railway> railway;
		Direction direction;
		QList<std::shared_ptr<const RailStation>> stations;
	};

	/**
	 * Get the list of stations using dialog.
	 * Empty if nothing selected.
	 */
	static Result dlgGetStations(RailCategory& cat, QWidget* parent,
		const QString& title = tr("选择线路车站"), const QString& prompt = {});

private:
	void initUI(const QString& prompt);

private slots:

	/**
	 * Setup the table according to current data in m_cbRail and m_radioDirection.
	 */
	void setupTable();

public slots:
	void accept()override;
};