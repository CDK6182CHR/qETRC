#include "paintstationinfowidget.h"

#include <QLabel>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QTime>
#include <QApplication>
#include <QStyle>

#include "data/rail/railstation.h"
#include "data/common/stationname.h"
#include "util/utilfunc.h"
#include "defines/icon_specs.h"


PaintStationInfoWidget::PaintStationInfoWidget(int id, std::shared_ptr<const RailStation> station, DiagramWidget* d,
	const QString& trainName, bool greedy, QWidget* parent):
	QWidget(parent), _id(id), _station(station), _diagramWidget(d), _greedyPaint(greedy)
{
	initUI(trainName);
}

int PaintStationInfoWidget::stopSeconds() const
{
	return spMin->value() * 60 + spSec->value();
}

void PaintStationInfoWidget::closeEvent(QCloseEvent* event)
{
	emit widgetClosed();
	QWidget::closeEvent(event);
}

void PaintStationInfoWidget::initUI(const QString& trainName)
{
	auto* vlay = new QVBoxLayout(this);
	auto* lab = new QLabel(tr("正在铺画 <font size=5>%1</font>次  "
		"<font size=5>%2</font> 站").arg(trainName, _station->name.toSingleLiteral()));
	labTitle = lab;
	auto* hlay = new QHBoxLayout;
	hlay->addWidget(lab);
	hlay->addStretch(2);
	auto* btn = new QToolButton();
	btn->setIcon(QEICN_paint_station_info_close);
	connect(btn, &QToolButton::clicked, this, &PaintStationInfoWidget::close);
	hlay->addWidget(btn);
	vlay->addLayout(hlay);

	auto* form = new QFormLayout();
	spMin = new QSpinBox;
	spMin->setRange(0, 10000);
	spMin->setSuffix(tr("分 (min)"));
	spSec = new QSpinBox;
	spSec->setRange(0, 10000);
	spSec->setSuffix(tr("秒 (s)"));
	spSec->setSingleStep(10);
	
	ckFix = new QCheckBox(tr("固定"));
	ckFix->setEnabled(_greedyPaint);

	connect(spMin, &QSpinBox::valueChanged, this, &PaintStationInfoWidget::actStopTimeSettingChanged);
	connect(spSec, &QSpinBox::valueChanged, this, &PaintStationInfoWidget::actStopTimeSettingChanged);
	connect(ckFix, &QCheckBox::stateChanged, this, &PaintStationInfoWidget::actFixChanged);

	hlay = new QHBoxLayout;
	hlay->addWidget(spMin);
	hlay->addWidget(spSec);
	hlay->addWidget(ckFix);
	form->addRow(tr("设置停时"), hlay);

	labStop = new QLabel;
	form->addRow(tr("实排时刻"), labStop);
	vlay->addLayout(form);
}

void PaintStationInfoWidget::onDataChanged(const QString& trainName, int stop_secs_set, int stop_secs_real, bool fix, 
	const QTime& arr, const QTime& dep)
{
	labTitle->setText(tr("正在铺画 <font size=5>%1</font>次  "
		"<font size=5>%2</font> 站").arg(trainName, _station->name.toSingleLiteral()));
	if (stopSeconds() != stop_secs_set) {
		// change by remote
		spMin->setValue(stop_secs_set / 60);
		spSec->setValue(stop_secs_set % 60);
	}
	if (fix != ckFix->isChecked()) {
		ckFix->setChecked(fix);
	}
	if (stop_secs_real) {
		labStop->setText(tr("<font size=5>%1/%2</font>  停车 <font size=5>%3</font>").arg(
			arr.toString("hh:mm:ss"), dep.toString("hh:mm:ss"), qeutil::secsToString(stop_secs_real)
		));
	}
	else {
		labStop->setText(tr("<font size=5>%1/...</font>").arg(
			arr.toString("hh:mm:ss")
		));
	}
}

void PaintStationInfoWidget::actStopTimeSettingChanged()
{
	emit stopTimeChanged(_id, stopSeconds());
}

void PaintStationInfoWidget::actFixChanged()
{
	emit fixStatusChanged(_id, ckFix->isChecked());
}
