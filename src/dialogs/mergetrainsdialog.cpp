#include "mergetrainsdialog.h"

#include <optional>

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QMessageBox>
#include <QDialogButtonBox>

#include "editors/train/selecttrainwidget.h"
#include "model/train/trainliststdmodel.h"
#include "data/train/train.h"
#include "data/train/traincollection.h"

MergeTrainsDialog::MergeTrainsDialog(TrainCollection& coll, QWidget* parent):
	QDialog(parent),
	m_coll(coll)
{
	initUI();
	resize(800, 800);
	setWindowTitle(tr("合并车次"));
}

void MergeTrainsDialog::accept()
{
	auto trains = m_selWidget->trains();
	if (trains.empty()) {
		QMessageBox::warning(this, tr("错误"), tr("未选择任何列车"));
		return;
	}

	std::optional<TrainName> newName;
	if (m_ckUseNewName->isChecked()) {
		// Check validity of new name
		newName = m_edNewName->text();
		if (!m_coll.trainNameIsValid(*newName, trains.front())) {
			QMessageBox::warning(this, tr("错误"), tr("所给的新车次非法"));
			return;
		}
	}

	auto newTrain = Train::mergeTrains(trains, newName, m_ckMergeCross->isChecked());
	emit mergeApplied(std::move(trains), std::move(newTrain));
	QDialog::accept();
}

void MergeTrainsDialog::reject()
{
	auto flag = QMessageBox::question(this, tr("提示"), tr("如果退出，已编辑的内容不会保存，是否继续？"));
	if (flag != QMessageBox::Yes)
		return;
	QDialog::reject();
}

void MergeTrainsDialog::initUI()
{
	auto* vlay = new QVBoxLayout(this);
	auto* lab = new QLabel(tr("请在下表中按顺序编辑所需合并的列车。所选列车将被合并为新的单一列车。"
		"新列车默认采取首列车的车次及类型。默认自动处理交叉站时刻。"));
	lab->setWordWrap(true);
	vlay->addWidget(lab);

	auto* hlay = new QHBoxLayout;
	m_ckUseNewName = new QCheckBox(tr("指定新车次名："));
	hlay->addWidget(m_ckUseNewName);
	m_edNewName = new QLineEdit;
	hlay->addWidget(m_edNewName);
	m_edNewName->setEnabled(false);
	vlay->addLayout(hlay);

	connect(m_ckUseNewName, &QCheckBox::toggled, [this](bool on) {
		m_edNewName->setEnabled(on);
		});

	m_ckMergeCross = new QCheckBox(tr("合并相邻列车的交叉站"));
	m_ckMergeCross->setChecked(true);
	vlay->addWidget(m_ckMergeCross);

	m_selWidget = new SelectTrainWidget(m_coll);
	vlay->addWidget(m_selWidget);

	auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	vlay->addWidget(box);

	connect(box, &QDialogButtonBox::accepted, this, &MergeTrainsDialog::accept);
	connect(box, &QDialogButtonBox::rejected, this, &MergeTrainsDialog::reject);
}
