#pragma once

#include <QDialog>

class Diagram;
class QSpinBox;
struct Config;

class DiagramOptionDialog : public QDialog
{
	Q_OBJECT
	Diagram& m_diagram;
	QSpinBox* m_spPassedStations;

public:
	DiagramOptionDialog(Diagram& diagram, QWidget* parent = nullptr);

private:
	void initUI();
	void setData();

signals:
	void passedStationChanged(int old_passed_stations, int new_passed_stations);

public slots:

	void accept()override;
};

