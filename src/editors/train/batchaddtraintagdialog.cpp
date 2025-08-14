#include "batchaddtraintagdialog.h"

#include <vector>

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QMessageBox>
#include <QCompleter>

#include "util/buttongroup.hpp"
#include "data/train/train.h"
#include "data/train/traincollection.h"
#include "model/train/traintaglistdirectmodel.h"

BatchAddTrainTagDialog::BatchAddTrainTagDialog(TrainCollection& coll, TrainTagListDirectModel* completionModel, QWidget* parent):
	QDialog(parent), m_coll(coll), m_completionModel(completionModel), m_tagCompleter(new QCompleter(completionModel, this))
{
	setAttribute(Qt::WA_DeleteOnClose, true);
	initUI();
}

void BatchAddTrainTagDialog::initUI()
{
	setWindowTitle(tr("批量添加列车标签"));
	resize(800, 800);

	auto* vlay = new QVBoxLayout(this);
	auto* lab = new QLabel(tr("此功能可以一次性将同一标签添加到多个车次。输入的标签可以是存在标签，或者在此新建标签。"));
	lab->setWordWrap(true);
	vlay->addWidget(lab);

	auto* form = new QFormLayout;
	m_edTag = new QLineEdit;
	m_edTag->setCompleter(m_tagCompleter);
	form->addRow(tr("标签名"), m_edTag);

	m_ckCompleteOnly = new QCheckBox(tr("仅识别完整车次"));
	form->addRow(tr("选项"), m_ckCompleteOnly);

	vlay->addLayout(form);

	lab = new QLabel(tr("请在下方编辑框内输入车次，每行一个车次。"));
	lab->setWordWrap(true);
	vlay->addWidget(lab);

	m_edTrains = new QTextEdit;
	vlay->addWidget(m_edTrains);

	auto* g = new ButtonGroup<2>({ "确定", "关闭"});
	g->connectAll(SIGNAL(clicked()), this, { SLOT(actApply()), SLOT(close()) });
	vlay->addLayout(g);

	vlay->addWidget(new QLabel(tr("解析结果：")));
	m_edReport = new QTextEdit;
	m_edReport->setReadOnly(true);
	m_edReport->setMaximumHeight(150);
	vlay->addWidget(m_edReport);
}

void BatchAddTrainTagDialog::actApply()
{
	QString report;
	QString tagName = m_edTag->text();
	QString trainNameText = m_edTrains->toPlainText();

	if (tagName.isEmpty()) {
		QMessageBox::warning(this, tr("错误"), tr("标签名称不能为空，请重新输入！"));
		return;
	}
	if (trainNameText.isEmpty()) {
		QMessageBox::warning(this, tr("错误"), tr("未输入任何车次！"));
		return;
	}

	// It is safe to include one train multiple times, thus we do not check this
	std::vector<std::shared_ptr<Train>> trains;

	auto trainNames = trainNameText.split('\n');
	auto tag = m_coll.tagManager().find(tagName);   // possible null

	int ln = 0;
	foreach(const auto& tn_, trainNames) {
		++ln;
		auto tn = tn_.trimmed();
		if (tn.isEmpty())
			continue;

		auto t = m_coll.findFullName(tn);
		if (!t && !m_ckCompleteOnly->isChecked()) {
			t = m_coll.findFirstSingleName(tn);
		}

		if (!t) {
			report.append(tr("未找到车次%1 [输入第%2行]\n").arg(tn).arg(ln));
		}
		else {
			if (tag && t->hasTag(tag)) {
				report.append(tr("车次%1已有标签%2\n").arg(tn, tagName));
			}
			else {
				trains.emplace_back(std::move(t));
			}
		}
	}

	if (report.isEmpty()) {
		report.prepend("------------------\n");
	}

	if (trains.empty()) {
		report.prepend(tr("没有符合输入的车次\n"));
	}
	else {
		emit tagAdded(tagName, trains);
		report.prepend(tr("已将标签添加到%1个车次\n").arg(trains.size()));
	}
	m_edReport->setPlainText(report);
}
