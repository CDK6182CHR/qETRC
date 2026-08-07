#pragma once

#include <QWidget>
#include "buttongroup.hpp"

class QSpinBox;

class ShiftTimeWidget : public QWidget
{
	Q_OBJECT;
	RadioButtonGroup<2>* m_radioGroup;
	QSpinBox* m_spMin, * m_spSec;

public:
	ShiftTimeWidget(QWidget* parent = nullptr);

	/**
	 * The time to be shifted. Possibly negative (for earlier).
	 */
	int shiftSeconds() const;

	/**
	 * Get the shift seconds from a modal dialog.
	 * Return 0 if canceled. The ok parameter is set to true if the user pressed OK, false if canceled. If it is nullptr, do nothing.
	 */
	static int dlgGetShiftSeconds(QWidget* parent, const QString& title, const QString& prompt, bool* ok);

private:
	void initUI();
};