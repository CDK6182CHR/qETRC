#include "trainpenwidget.h"

#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QColorDialog>
#include <QLabel>
#include <QDialogButtonBox>

#include "util/linestylecombo.h"
#include "data/train/train.h"

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

void TrainPenWidget::setStyle(std::shared_ptr<Train> train)
{
    setStyle(train->autoStyle(), train->lineStyle());
}

void TrainPenWidget::setStyle(bool isAuto, const TrainLineStyle& style)
{
    auto& pen = style.line_pen();
    m_ckAuto->setChecked(isAuto);
    m_btnColor->setText(pen.color().name());
    m_spWidth->setValue(pen.widthF());
    m_cbStyle->setCurrentIndex(static_cast<int>(pen.style()));
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
		// TODO: marker style
		return TrainLineStyle(p, LineMarkerStyle::no_marker());
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

    m_btnColor = new QPushButton;
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
    hlay->addWidget(m_cbStyle);

    connect(m_ckAuto, &QCheckBox::toggled, [this](bool on) {
        m_btnColor->setEnabled(!on);
        m_spWidth->setEnabled(!on);
        m_cbStyle->setEnabled(!on);
        });
}
