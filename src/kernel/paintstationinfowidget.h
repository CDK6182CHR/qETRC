#pragma once

#include <QWidget>

class QLabel;
class QSpinBox;
class RailStation;
class TrainTime;
class QCheckBox;
class DiagramWidget;

/**
 * 2024.02.12  For the painting train line (with greedy paint or ruler paint),
 * shown on clicking of the station point.
 * Shows detailed information of the specified station, with possible adjusting stop time.
 */
class PaintStationInfoWidget : public QWidget
{
	Q_OBJECT;
	int _id;   // the row number in the paiting wizard
	std::shared_ptr<const RailStation> _station;
	DiagramWidget* _diagramWidget;
	bool _greedyPaint;
	QCheckBox* ckFix;
	QLabel* labTitle, * labStop;
	QSpinBox* spMin, * spSec;

public:
	PaintStationInfoWidget(int id, std::shared_ptr<const RailStation> station, DiagramWidget* diagramWidget,
		const QString& trainName, bool greedy,
		QWidget* parent = nullptr);
	auto id()const { return _id; }
	auto station()const { return _station; }
	int stopSeconds()const;
	auto* diagramWidget()const { return _diagramWidget; }
protected:
	virtual void closeEvent(QCloseEvent* event) override;
private:
	void initUI(const QString& trainName);
signals:
	void stopTimeChanged(int id, int secs);
	void fixStatusChanged(int id, bool on);
	void widgetClosed();
public slots:
	void onDataChanged(const QString& trainName, int stop_secs_set, int stop_secs_real, bool fix, const TrainTime& arr, const TrainTime& dep);
private slots:
	void actStopTimeSettingChanged();
	void actFixChanged();
};
