#pragma once

#include <QWidget>

#include <optional>

#include "data/train/trainlinestyle.h"

class PenStyleCombo;
class QDoubleSpinBox;
class QCheckBox;
class QPushButton;
class QComboBox;
class QPen;
class Train;
class TrainLineStyle;

/**
 * 2025.01.31  The widget that setting train pen style.
 * Previously part of EditTrainWidget.
 */
class TrainPenWidget : public QWidget
{
	Q_OBJECT
	QCheckBox* m_ckAuto;
	QDoubleSpinBox* m_spWidth;
	PenStyleCombo* m_cbStyle;
	QPushButton* m_btnColor;

	QComboBox* m_cbMarker;
	QPushButton* m_btnMarkerColor;
	QDoubleSpinBox* m_spMarkerSize;

	QColor m_color;
	QColor m_markerColor;

public:
	TrainPenWidget(QWidget* parent = nullptr);
	TrainPenWidget(std::shared_ptr<Train> train, QWidget* parent = nullptr);

	void setStyle(std::shared_ptr<Train> train);
	void setStyle(bool isAuto, const TrainLineStyle& style);

	std::optional<TrainLineStyle> lineStyle()const;

	struct GetPenReturnType {
		bool accepted = false;
		std::optional<TrainLineStyle> lineStyle = std::nullopt;
	};

	static GetPenReturnType getLineStyle(QWidget* parent, std::shared_ptr<Train> initPenTrain,
		const QString& title, const QString& prompt = {});

private:
	void initUI();
	void updateEnabled();
};
