#include "trainpenwidget.h"

#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QColorDialog>
#include <QLabel>
#include <QDialogButtonBox>

#include "util/linestylecombo.h"
#include "data/train/train.h"
#include "data/common/qesystem.h"

TrainPenWidget::TrainPenWidget(QWidget* parent):
    QWidget(parent)
{
	initUI();
}

TrainPenWidget::TrainPenWidget(std::shared_ptr<Train> train, QWidget* parent):
    TrainPenWidget(parent)
{
    setStyle(train);
}

TrainPenWidget::TrainPenWidget(const TrainLineStyle& style, QWidget* parent):
    TrainPenWidget(parent)
{
    setStyle(false, style);
    m_ckAuto->setEnabled(false);
}

void TrainPenWidget::setStyle(std::shared_ptr<Train> train)
{
    setStyle(train->autoStyle(), train->lineStyle());
}

void TrainPenWidget::setStyle(bool isAuto, const TrainLineStyle& style)
{
    auto& pen = style.line_pen();
    m_ckAuto->setChecked(isAuto);
    m_color = pen.color();
    m_btnColor->setText(m_color.name());
    m_spWidth->setValue(pen.widthF());
    m_cbStyle->setCurrentIndex(static_cast<int>(pen.style()));

    const auto& marker = style.marker();
    m_cbMarker->setCurrentIndex(static_cast<int>(marker.type()));
    m_markerColor = marker.pen().color();
    if (marker.type() == LineMarkerType::None || !m_markerColor.isValid())
        m_markerColor = m_color;
    m_btnMarkerColor->setText(m_markerColor.name());
    if (marker.marker_size() > 0)
        m_spMarkerSize->setValue(marker.marker_size());

    updateEnabled();
}

std::optional<TrainLineStyle> TrainPenWidget::lineStyle() const
{
    if (m_ckAuto->isChecked()) {
        return std::nullopt;
    }
    else {
        QPen p;
        p.setColor(m_color);
        p.setWidthF(m_spWidth->value());
        p.setStyle(static_cast<Qt::PenStyle>(m_cbStyle->currentIndex()));

        if (p.style() == Qt::CustomDashLine) {
            p.setDashPattern(SystemJson::get().dash_line_pattern());
        }

        auto type = static_cast<LineMarkerType>(m_cbMarker->currentData().toInt());
        LineMarkerStyle marker = LineMarkerStyle::no_marker();
        if (type != LineMarkerType::None) {
            // The marker uses the line width and style, but has its own color.
            QPen markerPen = p;
            markerPen.setColor(m_markerColor);
            marker = LineMarkerStyle(type, markerPen, m_spMarkerSize->value());
        }
		return TrainLineStyle(p, std::move(marker));
    }
}

TrainPenWidget::GetPenReturnType TrainPenWidget::getLineStyle(QWidget* parent, std::shared_ptr<Train> initPenTrain, 
    const QString& title, const QString& prompt)
{
    QDialog* dlg = new QDialog(parent);
    dlg->setWindowTitle(title);
    auto* vlay = new QVBoxLayout(dlg);
    if (!prompt.isEmpty()) {
        auto* lab = new QLabel(prompt);
        lab->setWordWrap(true);
        vlay->addWidget(lab);
    }

    auto* w = new TrainPenWidget(initPenTrain, parent);
    vlay->addWidget(w);
    
    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    vlay->addWidget(box);

    connect(box, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

    bool flag = dlg->exec();
    GetPenReturnType res;
    if (flag) {
        // accepted
        res.accepted = true;
        res.lineStyle = w->lineStyle();
    }
    else {
        res.accepted = false;
    }

    // Clean up
    dlg->setParent(nullptr);
    dlg->deleteLater();
    return res;
}

void TrainPenWidget::initUI()
{
	auto* hlay = new QHBoxLayout(this);
    m_ckAuto = new QCheckBox(tr("自动"));
    hlay->addWidget(m_ckAuto);

    m_color = Qt::black;
    m_btnColor = new QPushButton(m_color.name());
    m_btnColor->setToolTip(tr("运行线颜色"));
    hlay->addWidget(m_btnColor);
    connect(m_btnColor, &QPushButton::clicked, [this]() {
        auto color = QColorDialog::getColor(m_color, this, tr("选择运行线颜色"));
        if (color.isValid()) {
            m_color = color;
            m_btnColor->setText(m_color.name());
        }
        });

    m_spWidth = new QDoubleSpinBox;
    m_spWidth->setPrefix(tr("宽度 "));
    m_spWidth->setSingleStep(0.5);
    m_spWidth->setRange(0, 100);
    m_spWidth->setDecimals(2);
    hlay->addWidget(m_spWidth);

    m_cbStyle = new PenStyleCombo;
    m_cbStyle->setToolTip(tr("运行线线型"));
    hlay->addWidget(m_cbStyle);

    hlay->addWidget(new QLabel(tr("标记")));
    m_cbMarker = new QComboBox;
    for (int i = 0; i < static_cast<int>(LineMarkerType::MAX); ++i) {
        m_cbMarker->addItem(line_marker_type_names.at(i), i);
    }
    hlay->addWidget(m_cbMarker);

    m_markerColor = Qt::black;
    m_btnMarkerColor = new QPushButton(m_markerColor.name());
    m_btnMarkerColor->setToolTip(tr("标记颜色"));
    hlay->addWidget(m_btnMarkerColor);
    connect(m_btnMarkerColor, &QPushButton::clicked, [this]() {
        auto color = QColorDialog::getColor(m_markerColor, this, tr("选择标记颜色"));
        if (color.isValid()) {
            m_markerColor = color;
            m_btnMarkerColor->setText(m_markerColor.name());
        }
        });

    m_spMarkerSize = new QDoubleSpinBox;
    m_spMarkerSize->setPrefix(tr("标记尺寸 "));
    m_spMarkerSize->setSingleStep(1);
    m_spMarkerSize->setRange(0, 100);
    m_spMarkerSize->setDecimals(2);
    m_spMarkerSize->setValue(6.0);
    hlay->addWidget(m_spMarkerSize);

    connect(m_ckAuto, &QCheckBox::toggled, this, &TrainPenWidget::updateEnabled);
    connect(m_cbMarker, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &TrainPenWidget::updateEnabled);
    updateEnabled();
}

void TrainPenWidget::updateEnabled()
{
    const bool isAuto = m_ckAuto->isChecked();
    m_btnColor->setEnabled(!isAuto);
    m_spWidth->setEnabled(!isAuto);
    m_cbStyle->setEnabled(!isAuto);
    m_cbMarker->setEnabled(!isAuto);

    const bool hasMarker = !isAuto &&
        static_cast<LineMarkerType>(m_cbMarker->currentData().toInt()) != LineMarkerType::None;
    m_btnMarkerColor->setEnabled(hasMarker);
    m_spMarkerSize->setEnabled(hasMarker);
}
