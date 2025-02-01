#pragma once

#include <QWidget>

#include <optional>
#include <QPen>

class PenStyleCombo;
class QDoubleSpinBox;
class QCheckBox;
class QPushButton;
class QPen;
class Train;

/**
 * 2025.01.31  The widget that setting train pen style.
 * Previously part of EditTrainWidget.
 */
class TrainPenWidget : public QWidget
{
	QCheckBox* m_ckAuto;
	QDoubleSpinBox* m_spWidth;
	PenStyleCombo* m_cbStyle;
	QPushButton* m_btnColor;

	QColor m_color;

public:
	TrainPenWidget(QWidget* parent = nullptr);
	TrainPenWidget(std::shared_ptr<Train> train, QWidget* parent = nullptr);

	void setPen(std::shared_ptr<Train> train);
	void setPen(bool isAuto, const QPen& pen);

	std::optional<QPen> pen()const;

	struct GetPenReturnType {
		bool accepted = false;
		std::optional<QPen> pen = std::nullopt;
	};

	static GetPenReturnType getPen(QWidget* parent, std::shared_ptr<Train> initPenTrain,
		const QString& title, const QString& prompt = {});

private:
	void initUI();
};
