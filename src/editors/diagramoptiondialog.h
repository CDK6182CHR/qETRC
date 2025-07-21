#pragma once

#include <QDialog>
#include <QUndoCommand>

class Diagram;
class QSpinBox;
class MainWindow;
struct Config;

class DiagramOptionDialog : public QDialog
{
	Q_OBJECT
	Diagram& m_diagram;
	QSpinBox* m_spPassedStations;
	QSpinBox* m_spPeriodHours;

public:
	DiagramOptionDialog(Diagram& diagram, QWidget* parent = nullptr);

private:
	void initUI();
	void setData();

signals:
	void passedStationChanged(int old_passed_stations, int new_passed_stations);
	void periodHoursChanged(int old_period_hours, int new_period_hours);

public slots:

	void accept()override;
};

namespace qecmd {

	class ChangePeriodHours : public QUndoCommand
	{
		Diagram& m_diagram;
		MainWindow* const m_mw;
		int m_old_period_hours, m_new_period_hours;
	public:
		ChangePeriodHours(Diagram& diagram, MainWindow* mw, int old_period_hours, int new_period_hours, QUndoCommand* parent = nullptr)
			: QUndoCommand(QObject::tr("更改周期小时数为%1").arg(new_period_hours), parent), 
			m_diagram(diagram), m_mw(mw), m_old_period_hours(old_period_hours), m_new_period_hours(new_period_hours) {
		}

		void undo()override;
		void redo()override;
	};
}

