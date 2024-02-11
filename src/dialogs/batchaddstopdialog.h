#pragma once
#include <QDialog>
#include <optional>
#include "util/buttongroup.hpp"

class QCheckBox;
class QSpinBox;


/**
 * 2024.02.11  Batch add stop at selected stations, for ruler paint and greedy paint
 */
class BatchAddStopDialog : public QDialog
{
	Q_OBJECT
	RadioButtonGroup<2>* rdRange;
	QCheckBox* ckLevel;
	QSpinBox* spMin, * spSec, * spLevel;

public:
	struct BatchAddStopReport {
		bool constr_range, constr_level;
		int stop_secs;
		int lowest_level;
	};

	BatchAddStopDialog(QWidget* parent = nullptr);

	static std::optional<BatchAddStopReport> setBatchAdd(QWidget* parent);

private:
	void initUI();

};
